#ifndef AppS_H
#define AppS_H

#include <map>
#include "MatrixDisplayUi.h"
#include "effects.h"

class IconContainer
{
public:
    IconContainer()
    {
        currentFrame = 0;
        isGif=false;
    }
    IconContainer(const IconContainer& src)
    {
        operator=(src);
    }
    virtual ~IconContainer()
    {}

    bool load(void)
    {
        if(icon)
            return true;

        if(iconName.length() == 0)
            return false;
        
        isGif=false;
        currentFrame = 0;
        icon.close();

        const char *extensions[] = {".jpg", ".gif"};
        bool isGifFlags[] = {false, true};

        for (int i = 0; i < 2; i++)
        {
            String filePath = "/ICONS/" + iconName + extensions[i];
            if (LittleFS.exists(filePath))
            {
                isGif = isGifFlags[i];
                icon = LittleFS.open(filePath);
                currentFrame = 0;

                ESP_LOGE("icons", "icon(%s) loaded %s", filePath.c_str(), icon ? "OK":"FAIL");
                return true;
            }
        }
        return false;
    }

    void play_ready(void)
    {
        currentFrame = 0;
        icon.close();
    }

    void clear(void)
    {
        isGif=false;
        currentFrame = 0;
        iconName.clear();
        icon.close();
    }

    IconContainer& operator=(const IconContainer& src)
    {
        iconName = src.iconName;

        icon = src.icon;
        isGif = src.isGif;
        currentFrame = src.currentFrame;
        return *this;
    }

    uint32_t draw_icon(GifPlayer* gp, int x, int y)
    {
        if(icon == false)
            return 0;

        if (isGif)
        {
            uint32_t iconWidth = gp->playGif(x, y, &icon, currentFrame);
            currentFrame = gp->getFrame();

            return iconWidth;
        }

        DisplayManager.drawJPG(x, y, icon);
        return 8;
    }

    String  iconName;

    File    icon;
    bool    isGif;
    uint8_t currentFrame;
};

struct CustomApp : app_base
{
    virtual void do_the_app(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer) override;

    int bounceDir = 0;
    bool hasCustomColor = false;
    String drawInstructions;
    float scrollposition = 0;
    int16_t scrollDelay = 0;
    byte lifetimeMode = 0;
    String text;
    bool bounce = false;
    uint32_t color;
    bool rainbow;
    bool center;
    int fade = 0;
    int blink = 0;
    int effect = -1;
    long duration = 0;
    byte textCase = 0;
    int16_t repeat = 0;
    int16_t currentRepeat = 0;
    OverlayEffect overlay;
    byte pushIcon = 0;
    float iconPosition = 0;
    bool iconWasPushed = false;
    int barData[16] = {0};
    uint32_t barBG = 0;
    int lineData[16] = {0};
    int gradient[2] = {0};
    int barSize;
    int lineSize;
    long lastUpdate;
    uint64_t lifetime;
    std::vector<uint32_t> colors;
    std::vector<String> fragments;
    int textOffset;
    int iconOffset;
    int progress = -1;
    uint32_t pColor;
    uint32_t background = 0;
    uint32_t pbColor;
    float scrollSpeed = 100;
    bool topText = true;
    bool noScrolling = true;
    bool lifeTimeEnd = false;
    uint8_t jpegDataBuffer[1000];
    unsigned int jpegDataSize = 0;

    IconContainer   icons[MAX_ICONS_PER_SCREEN];
};

extern std::vector<std::pair<String, app_base*>> Apps;
extern String currentCustomApp;
extern std::map<String, CustomApp*> customApps;
//extern uint32_t (*customAppCallbacks[20])(GenericLedMatrixIF *, MatrixDisplayUiState *, int16_t, int16_t, GifPlayer *);

#if defined ESP32_S3 || !defined USE_HUB75
    #define CUSTOMAPP_COUNT 20
#else
    #define CUSTOMAPP_COUNT 4
#endif     

extern CustomApp customAppMem[CUSTOMAPP_COUNT];

CustomApp *getCustomAppByName(String name);

//String getAppNameByFunction(AppCallback AppFunction);

String getAppNameAtIndex(int index);

int findAppIndexByName(const String &name);

const char *getTimeFormat();

typedef struct NativeApp : app_base
{
    typedef uint32_t (*native_app_t)(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);

    native_app_t native_app;

    NativeApp(native_app_t app, String name);

    virtual void do_the_app(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer) override;
}NativeApp;

uint32_t TimeApp(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t DateApp(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t TempApp(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t HumApp(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);

#ifdef ULANZI
    uint32_t BatApp(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
#endif

extern NativeApp* nativeAppList[];

/*
// Unattractive to have a function for every customapp which does the same, but currently still no other option found TODO
uint32_t CApp1(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp2(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp3(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp4(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp5(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp6(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp7(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp8(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp9(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp10(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp11(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp12(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp13(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp14(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp15(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp16(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp17(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp18(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp19(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
uint32_t CApp20(GenericLedMatrixIF *matrix, MatrixDisplayUiState *state, int16_t x, int16_t y, GifPlayer *gifPlayer);
*/
#endif