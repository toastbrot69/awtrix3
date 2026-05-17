/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 by Daniel Eichhorn
 * Copyright (c) 2016 by Fabrice Weinberg
 * Copyright (c) 2023 by Stephan Muehl (Blueforcer)
 * Copyright (c) 2023 by Thorsten Pohlmann (toastbrot69): Varibale screen size and mult-icon/gif
 * Note: This old lib for SSD1306 displays has been extremely
 * modified for AWTRIX 3 and has nothing to do with the original purposes.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#include "MatrixDisplayUi.h"
#include "effects.h"
#include "Globals.h"
#include "effects.h"

IconDisplayer displayer[MAX_ICONS_PER_SCREEN*2];

IMPLEMENT_GIFPLAYER()

IconDisplayer::IconDisplayer()
{
  p_type = t_unk;
}

void IconDisplayer::setMatrix(GenericLedMatrixIF *matrix, uint32_t ix, bool alt)
{
  p_ix = ix;
  p_alt = alt;
  GifPlayer::setMatrix(matrix);
}

bool IconDisplayer::set(const String& iconname)
{
  if(p_type != t_unk)
  {
    ESP_LOGE("icons", "IconDisplayer[%u:%s]::set(%s): type is set", p_ix, p_alt? "1":"0", iconname.c_str());
    return false;
  }

  static const char *extensions[] = {".jpg", ".gif"};
  static type_e types[] = {t_jpg, t_gif};

  for (int i = 0; i < 2; i++)
  {
    p_filename = "/ICONS/" + iconname + extensions[i];
    if (LittleFS.exists(p_filename))
    {
        GifPlayer::file = LittleFS.open(p_filename);

        if(GifPlayer::file)
        {
          p_type = types[i];

          ESP_LOGE("icons", "IconDisplayer[%u:%s] icon(%s) loaded", p_ix, p_alt? "1":"0", p_filename.c_str());

          if(p_type == t_gif)
            GifPlayer::setGif();

          p_iconname = iconname;

          return true;
        }

        ESP_LOGE("icons", "icon(%s) failed!!", p_filename.c_str());
        break;
    }
  }

  ESP_LOGE("icons", "IconDisplayer[%u:%s]::set(%s): file not found", p_ix, p_alt? "1":"0", iconname.c_str());

  p_iconname.clear();
  p_filename.clear();
  return false;
}

void IconDisplayer::release(void)
{
  if(p_iconname.length() > 0)
    ESP_LOGE("icons", "IconDisplayer[%u:%s]::release(%s)", p_ix, p_alt? "1":"0", p_iconname.c_str());

  GifPlayer::file.close();
  p_filename.clear();
  p_iconname.clear();
  p_type = t_unk;
}

int IconDisplayer::display(int x, int y, uint8_t frame)
{
  int ret = 0;

  switch(p_type)
  {
    case t_gif:
      ret = GifPlayer::drawGif(x, y, frame);
    break;

    case t_jpg:
      DisplayManager.drawJPG(x, y, GifPlayer::file);
      ret = 8;
    break;

    default:
    break;
  }

  return ret;
}

void clearAllDisplayers(void)
{
  ESP_LOGE("icons", "clearAllDisplayers");

  for(int t = 0; t < MAX_ICONS_PER_SCREEN*2; t++)
  {
    displayer[t].release();
    //displayer[0][t].release();
    //displayer[1][t].release();
  }
}

IconDisplayer* getDisplayer(const String& iconname, bool arr2)
{
  //IconDisplayer* arr = arr2 ? displayer[1] : displayer[0];

  IconDisplayer* arr = displayer;
  int t;

  // gibts das schon?
  for(t = 0; t < MAX_ICONS_PER_SCREEN*2; t++)
  {
    if(arr[t].getIconName() == iconname)
    {
      return &arr[t];
    } 
  }

  for(t = 0; t < MAX_ICONS_PER_SCREEN*2; t++)
  {
    if(arr[t].isEmpty())
    {
      if(arr[t].set(iconname))
      {
        return &arr[t];
      }

      arr[t].release();
      break;
    }
  }

  ESP_LOGE("icons", "getDisplayer(%s) failed", iconname.c_str());

  return NULL;
}


///////////////////////////////////////////////////////////////////////

MatrixDisplayUi::MatrixDisplayUi(GenericLedMatrixIF *matrix)
{
  this->matrix = matrix;
}

void MatrixDisplayUi::init()
{
  this->matrix->begin();
  this->matrix->setTextWrap(false);
  this->matrix->setBrightness(70);

  for(int t = 0; t < MAX_ICONS_PER_SCREEN*2; t++)
  {
    displayer[t].setMatrix(this->matrix, t, false);
    //displayer[0][t].setMatrix(this->matrix, t, false);
    //displayer[1][t].setMatrix(this->matrix, t, true);
  }
}

void MatrixDisplayUi::setTargetFPS(uint8_t fps)
{
  float oldInterval = this->updateInterval;
  this->updateInterval = ((float)1.0 / (float)fps) * 1000;

  // Calculate new ticksPerApp
  float changeRatio = oldInterval / (float)this->updateInterval;
  // this->ticksPerApp *= changeRatio;
  this->ticksPerTransition *= changeRatio;
}

void MatrixDisplayUi::setBackgroundEffect(int effect)
{
  this->BackgroundEffect = effect;
}

// -/------ Automatic control ------\-

void MatrixDisplayUi::enablesetAutoTransition()
{
  this->setAutoTransition = true;
}
void MatrixDisplayUi::disablesetAutoTransition()
{
  this->setAutoTransition = false;
}
void MatrixDisplayUi::setsetAutoTransitionForwards()
{
  this->state.appTransitionDirection = 1;
  this->lastTransitionDirection = 1;
}
void MatrixDisplayUi::setsetAutoTransitionBackwards()
{
  this->state.appTransitionDirection = -1;
  this->lastTransitionDirection = -1;
}
void MatrixDisplayUi::setTimePerApp(long time)
{
  this->ticksPerApp = time / updateInterval;
}
void MatrixDisplayUi::setTimePerTransition(uint16_t time)
{
  this->ticksPerTransition = (int)((float)time / (float)updateInterval);
}

// -/----- App settings -----\-
void MatrixDisplayUi::setAppAnimation(AnimationDirection dir)
{
  this->appAnimationDirection = dir;
}

void MatrixDisplayUi::setApps(const std::vector<std::pair<String, app_base*>> &appPairs)
{
  delete[] AppBases;
  AppCount = appPairs.size();
  AppBases = new app_base*[AppCount];
  for (size_t i = 0; i < AppCount; ++i)
  {
    AppBases[i] = appPairs[i].second;
  }
  this->resetState();
  DisplayManager.sendAppLoop();
  DisplayManager.setAutoTransition(true);
}

// -/----- Overlays ------\-
void MatrixDisplayUi::setOverlays(OverlayCallback *overlayFunctions, uint8_t overlayCount)
{
  this->overlayFunctions = overlayFunctions;
  this->overlayCount = overlayCount;
}

void MatrixDisplayUi::setBackground(BackgroundCallback backgroundFunction)
{
  this->backgroundFunction = backgroundFunction;
}

// -/----- Manual control -----\-
void MatrixDisplayUi::nextApp()
{
  if (this->state.appState != IN_TRANSITION)
  {
    this->state.manualControl = true;
    this->state.setAppState(IN_TRANSITION);
    this->state.ticksSinceLastStateSwitch = 0;
    this->lastTransitionDirection = this->state.appTransitionDirection;
    this->state.appTransitionDirection = 1;
  }
}
void MatrixDisplayUi::previousApp()
{
  if (this->state.appState != IN_TRANSITION)
  {
    this->state.manualControl = true;
    this->state.setAppState(IN_TRANSITION);
    this->state.ticksSinceLastStateSwitch = 0;
    this->lastTransitionDirection = this->state.appTransitionDirection;
    this->state.appTransitionDirection = -1;
  }
}

bool MatrixDisplayUi::switchToApp(uint8_t app)
{
  if (app >= this->AppCount)
    return false;
  this->state.ticksSinceLastStateSwitch = 0;
  if (app == this->state.currentApp)
    return false;
  this->state.setAppState(FIXED);
  this->state.currentApp = app;
  return true;
}

void MatrixDisplayUi::transitionToApp(uint8_t app)
{
  if (app >= this->AppCount)
    return;
  this->state.ticksSinceLastStateSwitch = 0;
  if (app == this->state.currentApp)
    return;
  this->nextAppNumber = app;
  this->lastTransitionDirection = this->state.appTransitionDirection;
  this->state.manualControl = true;
  this->state.setAppState(IN_TRANSITION);
  this->state.appTransitionDirection = app < this->state.currentApp ? -1 : 1;
}

// -/----- State information -----\-
MatrixDisplayUiState *MatrixDisplayUi::getUiState()
{
  return &this->state;
}

int8_t MatrixDisplayUi::update()
{
  long appStart = millis();
  int took = 0;

  int8_t timeBudget = this->updateInterval - (appStart - this->state.lastUpdate);
  if (timeBudget <= 0)
  {
    // Implement frame skipping to ensure time budget is kept
    if (this->setAutoTransition && this->state.lastUpdate != 0)
      this->state.ticksSinceLastStateSwitch += ceil(-timeBudget / this->updateInterval);

    this->state.lastUpdate = appStart;
    this->tick();

    took = (millis() - appStart);
    //ESP_LOGE("displ", "update: %i\n", took);
  }
  
  return this->updateInterval - took;
}

void MatrixDisplayUi::tick()
{
  this->state.ticksSinceLastStateSwitch++;

  if (this->AppCount > 0)
  {
    switch (this->state.appState)
    {
    case IN_TRANSITION:
      if (this->state.ticksSinceLastStateSwitch >= this->ticksPerTransition)
      {
        this->state.setAppState(FIXED);
        this->state.currentApp = getnextAppNumber();
        this->state.ticksSinceLastStateSwitch = 0;
        this->nextAppNumber = -1;
      }
      break;
    case FIXED:
      // Revert manualControl
      if (this->state.manualControl)
      {
        this->state.appTransitionDirection = 1;
        this->state.manualControl = false;
      }
      if (this->state.ticksSinceLastStateSwitch >= this->ticksPerApp)
      {
        if (this->setAutoTransition)
        {
          this->state.setAppState(IN_TRANSITION);
        }
        this->state.ticksSinceLastStateSwitch = 0;
      }
      break;
    }
  }

  this->matrix->clear();

  if (BackgroundEffect > -1)
  {
    callEffect(this->matrix, 0, 0, BackgroundEffect);
  }

  if (this->AppCount > 0)
    this->drawApp();
  this->drawOverlays();
  this->drawIndicators();
  if (GLOBAL_OVERLAY > 0)
  {
    EffectOverlay(matrix, 0, 0, GLOBAL_OVERLAY);
  }
  DisplayManager.gammaCorrection();
  this->matrix->show();
}

void MatrixDisplayUi::drawIndicators()
{
  uint32_t drawColor;

  // Indicator 1
  if (indicator1State)
  {
    if (indicator1Blink)
    {
      if (millis() % (2 * indicator1Blink) < indicator1Blink)
      {
        drawColor = indicator1Color;
      }
      else
      {
        drawColor = 0; // Schwarz
      }
    }
    else if (indicator1Fade)
    {
      drawColor = fadeColor(indicator1Color, indicator1Fade);
    }
    else
    {
      drawColor = indicator1Color;
    }
    matrix->drawPixel(31, 0, drawColor);
    matrix->drawPixel(30, 0, drawColor);
    matrix->drawPixel(31, 1, drawColor);
  }

  // Indicator 2
  if (indicator2State)
  {
    if (indicator2Blink)
    {
      if (millis() % (2 * indicator2Blink) < indicator2Blink)
      {
        drawColor = indicator2Color;
      }
      else
      {
        drawColor = 0; // Schwarz
      }
    }
    else if (indicator2Fade)
    {
      drawColor = fadeColor(indicator2Color, indicator2Fade);
    }
    else
    {
      drawColor = indicator2Color;
    }
    matrix->drawPixel(31, 3, drawColor);
    matrix->drawPixel(31, 4, drawColor);
  }

  // Indicator 3
  if (indicator3State)
  {
    if (indicator3Blink)
    {
      if (millis() % (2 * indicator3Blink) < indicator3Blink)
      {
        drawColor = indicator3Color;
      }
      else
      {
        drawColor = 0; // Schwarz
      }
    }
    else if (indicator3Fade)
    {
      drawColor = fadeColor(indicator3Color, indicator3Fade);
    }
    else
    {
      drawColor = indicator3Color;
    }
    matrix->drawPixel(31, 7, drawColor);
    matrix->drawPixel(31, 6, drawColor);
    matrix->drawPixel(30, 7, drawColor);
  }
}

uint32_t MatrixDisplayUi::fadeColor(uint32_t color, uint32_t interval)
{
  float phase = (sin(2 * PI * millis() / float(interval)) + 1) * 0.5;
  uint8_t r = ((color >> 16) & 0xFF) * phase;
  uint8_t g = ((color >> 8) & 0xFF) * phase;
  uint8_t b = (color & 0xFF) * phase;
  return (r << 16) | (g << 8) | b;
}

uint8_t currentTransition;
bool gotNewTransition = true;
TransitionType getRandomTransition()
{
  // RANDOM is now index 0, so we add 1 to the result to ensure it's never selected
  return static_cast<TransitionType>((rand() % (CROSSFADE)) + 1);
}

bool swapped = false;

#include "Apps.h"

void MatrixDisplayUiState::setAppState(AppState a)
{
  if(a == appState)
    return;

  //clearAllDisplayers();

  appState = a;
}

void MatrixDisplayUi::drawApp()
{
  switch (this->state.appState)
  {
  case IN_TRANSITION:
  {
    swapped = false;
    gotNewTransition = false;
    if (currentTransition == SLIDE)
    {
      slideTransition();
    }
    else if (currentTransition == FADE)
    {
      fadeTransition();
    }
    else if (currentTransition == ZOOM)
    {
      zoomTransition();
    }
    else if (currentTransition == ROTATE)
    {
      rotateTransition();
    }
    else if (currentTransition == PIXELATE)
    {
      pixelateTransition();
    }
    else if (currentTransition == CURTAIN)
    {
      curtainTransition();
    }
    else if (currentTransition == RIPPLE)
    {
      rippleTransition();
    }
    else if (currentTransition == BLINK)
    {
      blinkTransition();
    }
    else if (currentTransition == RELOAD)
    {
      reloadTransition();
    }
    else if (currentTransition == CROSSFADE)
    {
      crossfadeTransition();
    }
    break;
  }
  case FIXED:
    if (TRANS_EFFECT == RANDOM)
    {
      if (gotNewTransition == false)
      {
        currentTransition = getRandomTransition(); // Wähle einen neuen zufälligen Übergang aus, wenn TRANS_EFFECT auf RANDOM gesetzt ist
        gotNewTransition = true;
      }
    }
    else
    {
      currentTransition = TRANS_EFFECT; // Wenn TRANS_EFFECT nicht RANDOM ist, setzen Sie currentTransition auf TRANS_EFFECT
    }

    uint8_t cur_app = this->state.currentApp;

    static uint8_t last_cur_app = cur_app;

    if(cur_app != last_cur_app)
    {
      clearAllDisplayers();

      last_cur_app = cur_app;
    }

    #if 0
      this->AppBases[this->state.currentApp]->do_the_app(this->matrix, &this->state, 0, 0, false);
    #else

      uint32_t used_h = 0;

      app_base* app = this->AppBases[cur_app];

      while(used_h == 0 || (used_h + app->height) <= MATRIX_HEIGHT)
      {
        //ESP_LOGE("displ", "draw(%s): y:%i -> %i\n", app->name.c_str(), used_h, used_h+app->height-1);
        this->state.lastAppOnScreen = cur_app;
        app->do_the_app(this->matrix, &this->state, 0, used_h, false);
        used_h += app->height;
        cur_app = (cur_app + AppCount + 1) % AppCount;
        app = this->AppBases[cur_app];
      }
    #endif
    swapped = true;
    break;
  }
}

bool MatrixDisplayUi::isCurrentAppValid()
{
  for (size_t i = 0; i < AppCount; ++i)
  {
    if (AppBases[i] == AppBases[this->state.currentApp])
    {
      return true;
    }
  }
  return false;
}

void MatrixDisplayUi::resetState()
{
  if (!isCurrentAppValid())
  {
    this->state.lastUpdate = 0;
    this->state.ticksSinceLastStateSwitch = 0;
    this->state.setAppState(FIXED);
    this->state.currentApp = 0;
  }
}

void MatrixDisplayUi::forceResetState()
{
  this->state.lastUpdate = 0;
  this->state.ticksSinceLastStateSwitch = 0;
  this->state.setAppState(FIXED);
  this->state.currentApp = 0;
}

void MatrixDisplayUi::drawOverlays()
{
  for (uint8_t i = 0; i < this->overlayCount; i++)
  {
    (this->overlayFunctions[i])(this->matrix, &this->state, true);
  }
}

void MatrixDisplayUi::drawBackground()
{
  this->backgroundFunction(this->matrix);
}

uint8_t MatrixDisplayUi::getnextAppNumber()
{
  if (this->nextAppNumber != -1)
    return this->nextAppNumber;

  if(this->state.appTransitionDirection > 0)
    return (this->state.lastAppOnScreen + this->AppCount + this->state.appTransitionDirection) % this->AppCount;

  return (this->state.currentApp + this->AppCount + this->state.appTransitionDirection) % this->AppCount;
}

void MatrixDisplayUi::setIndicator1Color(uint32_t color)
{
  this->indicator1Color = color;
}

void MatrixDisplayUi::setIndicator1State(bool state)
{
  this->indicator1State = state;
}

void MatrixDisplayUi::setIndicator1Blink(int blink)
{
  this->indicator1Blink = blink;
}

void MatrixDisplayUi::setIndicator1Fade(int fade)
{
  this->indicator1Fade = fade;
}

void MatrixDisplayUi::setIndicator2Color(uint32_t color)
{
  this->indicator2Color = color;
}

void MatrixDisplayUi::setIndicator2State(bool state)
{
  this->indicator2State = state;
}

void MatrixDisplayUi::setIndicator2Blink(int blink)
{
  this->indicator2Blink = blink;
}

void MatrixDisplayUi::setIndicator2Fade(int fade)
{
  this->indicator2Fade = fade;
}

void MatrixDisplayUi::setIndicator3Color(uint32_t color)
{
  this->indicator3Color = color;
}

void MatrixDisplayUi::setIndicator3State(bool state)
{
  this->indicator3State = state;
}

void MatrixDisplayUi::setIndicator3Blink(int blink)
{
  this->indicator3Blink = blink;
}

void MatrixDisplayUi::setIndicator3Fade(int fade)
{
  this->indicator3Fade = fade;
}

// ------------------ TRANSITIONS -------------------
float distance(int x1, int y1, int x2, int y2)
{
  return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void rotate(int &x, int &y, float angle)
{
  // Move the point to the origin
  x -= MATRIX_WIDTH/2;
  y -= MATRIX_HEIGHT/2;

  // Perform the rotation
  int newX = x * cos(angle) - y * sin(angle);
  int newY = x * sin(angle) + y * cos(angle);

  // Move the point back to the actual origin
  x = newX + MATRIX_WIDTH/2;
  y = newY + MATRIX_HEIGHT/2;
}

void MatrixDisplayUi::fadeTransition()
{
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;
  int fadeValue;
  if (progress < 0.5)
  {
    fadeValue = pow(progress * 2, 2) * 255; // Fading out the old app (progress from 0 to 0.5)
  }
  else
  {
    fadeValue = pow((1.0 - progress) * 2, 2) * 255; // Fading in the new app (progress from 0.5 to 1.0)
  }
  this->matrix->clear(); // Clear the matrix
  // If fading out the old app
  if (progress < 0.5)
  {
    (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, 0, 0, false);
  }
  else
  {
    // Otherwise fading in the new app
    (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, 0, 0, true);
  }

  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      CRGB color = DisplayManager.getLeds()[this->matrix->XY(i, j)];
      color.fadeToBlackBy(fadeValue);
      DisplayManager.getLeds()[this->matrix->XY(i, j)] = color;
    }
  }
}

void MatrixDisplayUi::slideTransition()
{
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;
  int16_t x, y, x1, y1;
  switch (this->appAnimationDirection)
  {
  case SLIDE_UP:
    x = 0;
    y = -MATRIX_HEIGHT * progress;
    x1 = 0;
    y1 = y + MATRIX_HEIGHT;
    break;
  case SLIDE_DOWN:
    x = 0;
    y = MATRIX_HEIGHT * progress;
    x1 = 0;
    y1 = y - MATRIX_HEIGHT;
    break;
  }
  // Invert animation if direction is reversed.
  int8_t dir = this->state.appTransitionDirection >= 0 ? 1 : -1;
  x *= dir;
  y *= dir;
  x1 *= dir;
  y1 *= dir;
  (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, x, y, false);
  (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, x1, y1, true);
}

void MatrixDisplayUi::curtainTransition()
{
  CRGB *leds = DisplayManager.getLeds();
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;
  int curtainWidth = (int)((MATRIX_WIDTH/2) * progress); // 16 ist die Hälfte der Matrix-Breite

  if (this->state.ticksSinceLastStateSwitch == 1 || this->state.ticksSinceLastStateSwitch == 0)
  {
    // Kopieren Sie die aktuelle App-Ansicht in ledsCopy
    (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, 0, 0, false);
    for (int i = 0; i < MATRIX_WIDTH; i++)
    {
      for (int j = 0; j < MATRIX_HEIGHT; j++)
      {
        ledsCopy[i + j * MATRIX_WIDTH] = leds[this->matrix->XY(i, j)];
      }
    }
  }
  // Zeichnen Sie die neue App-Ansicht
  (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, 0, 0, true);

  // Anwenden des Vorhang-Effekts basierend auf dem Fortschritt
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      if ((i < ((MATRIX_WIDTH/2) - curtainWidth)) || (i >= ((MATRIX_WIDTH/2) + curtainWidth)))
      {
        leds[this->matrix->XY(i, j)] = ledsCopy[i + j * MATRIX_WIDTH];
      }
    }
  }
}

void MatrixDisplayUi::zoomTransition()
{
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;
  float scale = 1.0;
  // If zooming out the old app
  if (progress < 0.5)
  {
    scale = 1 - progress * 2; // scale will change from 1.0 to 0.0
    (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, 0, 0, false);
  }
  else
  {
    // Otherwise zooming in the new app
    scale = (progress - 0.5) * 2; // scale will change from 0.0 to 1.0
    (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, 0, 0, true);
  }

  // Copy the data to the temporary array ledsCopy
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      ledsCopy[i + j * MATRIX_WIDTH] = DisplayManager.getLeds()[this->matrix->XY(i, j)];
    }
  }

  // Scale the data and copy back to the matrix
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      int iScaled = (MATRIX_WIDTH/2) + (i - (MATRIX_WIDTH/2)) * scale;
      int jScaled = (MATRIX_HEIGHT/2) + (j - (MATRIX_HEIGHT/2)) * scale;

      if (iScaled < 0)
        iScaled = 0;
      if (iScaled >= MATRIX_WIDTH)
        iScaled = MATRIX_WIDTH-1;
      if (jScaled < 0)
        jScaled = 0;
      if (jScaled >= MATRIX_HEIGHT)
        jScaled = MATRIX_HEIGHT-1;
      DisplayManager.getLeds()[this->matrix->XY(i, j)] = ledsCopy[iScaled + jScaled * MATRIX_WIDTH];
    }
  }
}

void MatrixDisplayUi::rotateTransition()
{
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;
  float angle = progress * 2 * PI; // Rotate 360 degrees over the transition

  // Determine which app to draw
  if (progress < 0.5)
  {
    // Rotate out the old app (progress from 0 to 0.5)
    (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, 0, 0, false);
  }
  else
  {
    // Rotate in the new app (progress from 0.5 to 1.0)
    (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, 0, 0, true);
  }

  // Copy the data to the temporary array ledsCopy
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      ledsCopy[i + j * MATRIX_WIDTH] = DisplayManager.getLeds()[this->matrix->XY(i, j)];
    }
  }

  // Rotate the data and copy back to the matrix
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      int iRotated = i;
      int jRotated = j;
      rotate(iRotated, jRotated, angle);

      if (iRotated < 0)
        iRotated = 0;
      if (iRotated >= MATRIX_WIDTH)
        iRotated = MATRIX_WIDTH-1;
      if (jRotated < 0)
        jRotated = 0;
      if (jRotated >= MATRIX_HEIGHT)
        jRotated = MATRIX_HEIGHT-1;

      DisplayManager.getLeds()[this->matrix->XY(i, j)] = ledsCopy[iRotated + jRotated * MATRIX_WIDTH];
    }
  }
}

void MatrixDisplayUi::pixelateTransition()
{
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;

  // Draw the old app and copy to ledsCopy
  (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, 0, 0, false);
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      ledsCopy[i + j * MATRIX_WIDTH] = DisplayManager.getLeds()[this->matrix->XY(i, j)];
    }
  }

  // Clear the screen and draw the new app
  this->matrix->clear();
  (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, 0, 0, true);

  // Apply the random pixel swap transition effect
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      // If the random number is greater than the progress, display the pixel from the old app
      if (random(255) > progress * 255)
      {
        DisplayManager.getLeds()[this->matrix->XY(i, j)] = ledsCopy[i + j * MATRIX_WIDTH];
      }
      // Otherwise, keep the pixel from the new app
    }
  }
}

void MatrixDisplayUi::rippleTransition()
{
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;

  // Draw the old app and copy to ledsCopy
  (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, 0, 0, false);
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      ledsCopy[i + j * MATRIX_WIDTH] = DisplayManager.getLeds()[this->matrix->XY(i, j)];
    }
  }

  // Clear the screen and draw the new app
  this->matrix->clear();
  (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, 0, 0, true);

  // Apply the checkerboard transition effect
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      // If the sum of i and j is an even number and the progress is less than 0.5, display the pixel from the old app
      if ((i + j) % 2 == 0 && progress < 0.5)
      {
        DisplayManager.getLeds()[this->matrix->XY(i, j)] = ledsCopy[i + j * MATRIX_WIDTH];
      }
      // If the sum of i and j is an odd number and the progress is more than 0.5, display the pixel from the old app
      else if ((i + j) % 2 != 0 && progress >= 0.5)
      {
        DisplayManager.getLeds()[this->matrix->XY(i, j)] = ledsCopy[i + j * MATRIX_WIDTH];
      }
      // Otherwise, keep the pixel from the new app
    }
  }
}

void MatrixDisplayUi::blinkTransition()
{
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;

  // Number of blinks during the transition
  int blinks = 3;

  // Calculate the current blink state (on or off) by considering the number of blinks and the progress
  bool blinkState = (int)(progress * blinks) % 2 == 0;

  // Depending on the blinkState and the progress, draw the old or the new app
  if (blinkState)
  {
    // If blinkState is true, draw the old app if progress is less than 0.5, otherwise draw the new app
    if (progress < 0.5)
    {
      (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, 0, 0, false);
    }
    else
    {
      (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, 0, 0, true);
    }
  }
  else
  {
    // If blinkState is false, clear the matrix (display off)
    this->matrix->clear();
  }
}

void MatrixDisplayUi::reloadTransition()
{
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;
  int visiblePixel;

  if (progress < 0.5)
  {

    (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, 0, 0, false);

    // Calculating pixel to be visible based on progress
    visiblePixel = MATRIX_WIDTH * (1.0 - (progress * 2));
    if (visiblePixel < 0)
      visiblePixel = 0;

    for (int i = visiblePixel; i < MATRIX_WIDTH; i++)
    {
      for (int j = 0; j < MATRIX_HEIGHT; j++)
      {
        // Turning the pixels off to create a fly out effect
        DisplayManager.getLeds()[this->matrix->XY(i, j)] = CRGB::Black;
      }
    }
  }
  else
  {
    // Draw the new app and let the pixels fly in
    (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, 0, 0, true);

    // Calculating pixel to be visible based on progress
    visiblePixel = MATRIX_WIDTH * ((progress - 0.5) * 2);
    if (visiblePixel > MATRIX_WIDTH)
      visiblePixel = MATRIX_WIDTH;

    for (int i = visiblePixel; i < MATRIX_WIDTH; i++)
    {
      for (int j = 0; j < MATRIX_HEIGHT; j++)
      {
        // Turning the pixels off to create a fly in effect
        DisplayManager.getLeds()[this->matrix->XY(i, j)] = CRGB::Black;
      }
    }
  }
}

void MatrixDisplayUi::crossfadeTransition()
{
  float progress = (float)this->state.ticksSinceLastStateSwitch / (float)this->ticksPerTransition;

  // Draw the old app
  (this->AppBases[this->state.currentApp])->do_the_app(this->matrix, &this->state, 0, 0, false);

  // Copy the old app data to ledsCopy array
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      ledsCopy[i + j * MATRIX_WIDTH] = DisplayManager.getLeds()[this->matrix->XY(i, j)];
    }
  }

  // Clear the matrix before drawing the new app
  this->matrix->fillScreen(0);

  // Draw the new app
  (this->AppBases[this->getnextAppNumber()])->do_the_app(this->matrix, &this->state, 0, 0, true);

  // Linearly interpolate between old and new pixel colors based on the progress
  for (int i = 0; i < MATRIX_WIDTH; i++)
  {
    for (int j = 0; j < MATRIX_HEIGHT; j++)
    {
      CRGB pixelOld = ledsCopy[i + j * MATRIX_WIDTH];
      CRGB pixelNew = DisplayManager.getLeds()[this->matrix->XY(i, j)];
      DisplayManager.getLeds()[this->matrix->XY(i, j)] = pixelOld.lerp8(pixelNew, progress * 255);
    }
  }
}
