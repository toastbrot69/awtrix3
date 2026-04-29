#include "LedMatrix.h"

#ifdef USE_HUB75

void GenericLedMatrixIF::clear()
{
    fillScreenRGB888(0, 0, 0);
}

void GenericLedMatrixIF::show()
{ 
}

void GenericLedMatrixIF::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
    // transprent -> black bkground
    GFX::drawBitmap<uint16_t>(x, y, bitmap, w, h, color);
}

int GenericLedMatrixIF::XY(int16_t x, int16_t y) // compat with FastLED code, returns 1D offset
{
    if( x >= MATRIX_WIDTH) 
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




#endif // #ifdef USE_HUB75
