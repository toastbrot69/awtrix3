#include "Globals.h"
#include "LedMatrix.h"

#ifdef USE_HUB75

HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
/*
HUB75_I2S_CFG mxconfig(
	MATRIX_WIDTH, // Module width
	MATRIX_HEIGHT, // Module height
	1, // chain length
	_pins // pin mapping
);
*/
HUB75_I2S_CFG mxconfig(
	MATRIX_WIDTH, // Module width
	MATRIX_HEIGHT, // Module height
	1, // chain length
	_pins, // pin mapping
  HUB75_I2S_CFG::FM6124, // shift_driver 
  HUB75_I2S_CFG::TYPE_DIRECT, //line_driver
  true, // double buffer 
  HUB75_I2S_CFG::HZ_16M, // clk_speed _i2sspeed
  1, // Anything > 1 seems to cause artefacts on ICS panels
  false,  // clockphase
  60,  // _min_refresh_rate
  PIXEL_COLOR_DEPTH_BITS_DEFAULT // _pixel_color_depth_bits
);

GenericLedMatrixIF::GenericLedMatrixIF(CRGB * crgb) : MatrixPanel_I2S_DMA(mxconfig)
{
    p_crgb = crgb;
} 

void GenericLedMatrixIF::clear()
{
    fillScreenRGB888(0, 0, 0);

    memset(p_crgb, 0, sizeof(CRGB)*MATRIX_WIDTH*MATRIX_HEIGHT);
}

void GenericLedMatrixIF::show()
{ 
    flipDMABuffer();
}

static void col2rgb(uint16_t color, CRGB& crgb)
{
     // 565
    crgb.r = (color >> 8) & 0xf8;
    crgb.g = (color >> 3) & 0xfc;
    crgb.b = (color << 3) & 0xf8;
}

void  GenericLedMatrixIF::drawPixel(int16_t x, int16_t y, uint32_t color) // overwrite adafruit implementation
{
    CRGB crgb(color);
//        ESP_LOGE("HUB75", "dp(%i:%i) 0x%04x %02x %02x %02x", x, y, color, crgb.r, crgb.g, crgb.b);
    this->drawPixel(x, y, crgb);
}

void  GenericLedMatrixIF::drawPixel(int16_t x, int16_t y, uint16_t color) // overwrite adafruit implementation
{
    CRGB crgb(color);
    col2rgb(color, crgb);

//        ESP_LOGE("HUB75", "dp(%i:%i) 0x%04x %02x %02x %02x", x, y, color, crgb.r, crgb.g, crgb.b);
    this->drawPixel(x, y, crgb);
}

void   GenericLedMatrixIF::drawPixel(int16_t x, int16_t y, const CRGB& crgb) // overwrite adafruit implementation
{
    MatrixPanel_I2S_DMA::drawPixelRGB888(x, y, crgb.r, crgb.g, crgb.b);

    p_crgb[XY(x, y)] = crgb;
}

void GenericLedMatrixIF::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
    GFX::drawBitmap<uint16_t>(x, y, bitmap, w, h, color);
}

int GenericLedMatrixIF::XY(int16_t x, int16_t y) // compat with FastLED code, returns 1D offset
{
    if(x < 0 || y < 0) 
        return 0;
        
    if(x >= MATRIX_WIDTH) 
        return 0;
    
    if( y >= MATRIX_HEIGHT) 
        return 0;

    return (y * MATRIX_WIDTH) + x + 1; // everything offset by one to compute out of bounds stuff - never displayed by ShowFrame()
}

uint16_t GenericLedMatrixIF::Color(uint8_t r, uint8_t g, uint8_t b)
{
    return  ((uint16_t)(r & 0xF8) << 8) |
            ((uint16_t)(g & 0xFC) << 3) |
                        (b         >> 3);
}

void GenericLedMatrixIF::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    CRGB crgb;
    col2rgb(color, crgb);

#ifdef NO_FAST_FUNCTIONS
    GFX::drawLine<uint16_t>(x, y, x, y + h, color);
#else
    this->drawFastVLine(x, y, h, crgb.r, crgb.g, crgb.b);
#endif    
}


void GenericLedMatrixIF::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    CRGB crgb;
    col2rgb(color, crgb);

#ifdef NO_FAST_FUNCTIONS
    GFX::drawLine<uint16_t>(x, y, x + w, y, color);
#else
   this-> drawFastHLine(x, y, w, crgb.r, crgb.g, crgb.b);
#endif    
}

#ifndef NO_FAST_FUNCTIONS
void GenericLedMatrixIF::drawFastVLine(int16_t x, int16_t y, int16_t h, uint8_t r, uint8_t g, uint8_t b)
{
    MatrixPanel_I2S_DMA::drawFastVLine(x, y, h, r, g, b);

    int ix = XY(x, y);

    if(h > 0)
    {
        while(h > 0)
        {
            --h;

            if(ix > 0 && ix < MATRIX_WIDTH*MATRIX_HEIGHT)
            {
                p_crgb[ix].r = r;
                p_crgb[ix].g = g;
                p_crgb[ix].b = b;
            }

            ix += MATRIX_WIDTH;
        }
    }
    else
    {
        while(h < 0)
        {
            ++h;
            if(ix > 0 && ix < MATRIX_WIDTH*MATRIX_HEIGHT)
            {
                p_crgb[ix].r = r;
                p_crgb[ix].g = g;
                p_crgb[ix].b = b;
            }            
            ix -= MATRIX_WIDTH;
        }

    }
}

void GenericLedMatrixIF::drawFastHLine(int16_t x, int16_t y, int16_t w, uint8_t r, uint8_t g, uint8_t b)
{
    MatrixPanel_I2S_DMA::drawFastHLine(x, y, w, r, g, b);

    if(w < 0)
    {
        w = -w;
        x = x - w;
    }

    if(x < 0)
        x = 0;
    if(x+w > MATRIX_WIDTH)
        w = MATRIX_WIDTH-x;

    int ix = XY(x, y);

    for(x = 0; x < w; x++)
    {
        p_crgb[ix].r = r;
        p_crgb[ix].g = g;
        p_crgb[ix].b = b;
        ++ix;
    }
}
#endif 

#endif // #ifdef USE_HUB75
