/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 by Thorsten Pohlmann (toastbrot69)
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

#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#ifdef USE_HUB75
    #define MATRIX_WIDTH 64 
    #define MATRIX_HEIGHT 32

    #ifdef ESP32_S3
        #include "platforms/esp32s3/esp32s3-default-pins.hpp"

        #define R1_PIN   R1_PIN_DEFAULT
        #define G1_PIN   G1_PIN_DEFAULT
        #define B1_PIN   B1_PIN_DEFAULT
        #define R2_PIN   R2_PIN_DEFAULT
        #define G2_PIN   G2_PIN_DEFAULT
        #define B2_PIN   B2_PIN_DEFAULT
        #define A_PIN    A_PIN_DEFAULT
        #define B_PIN    B_PIN_DEFAULT
        #define C_PIN    C_PIN_DEFAULT
        #define D_PIN    D_PIN_DEFAULT
        #define E_PIN    39
        #define LAT_PIN  LAT_PIN_DEFAULT
        #define OE_PIN   OE_PIN_DEFAULT
        #define CLK_PIN  CLK_PIN_DEFAULT
    #else        
        #if 0
            #define R1_PIN 25
            #define G1_PIN 26
            #define B1_PIN 27
            #define R2_PIN 14
            #define G2_PIN 12
            #define B2_PIN 13
            #define A_PIN 23
            #define B_PIN 19
            #define C_PIN 5
            #define D_PIN 17
            #define E_PIN 18
            #define LAT_PIN 4
            #define OE_PIN 15
            #define CLK_PIN 16
        #else
            #define R1_PIN -1
            #define G1_PIN -1
            #define B1_PIN -1
            #define R2_PIN -1
            #define G2_PIN -1
            #define B2_PIN -1
            #define A_PIN -1
            #define B_PIN -1
            #define C_PIN -1
            #define D_PIN -1
            #define E_PIN -1
            #define LAT_PIN -1
            #define OE_PIN -1
            #define CLK_PIN -1
        #endif
    #endif

    #include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

    class GenericLedMatrixIF : public MatrixPanel_I2S_DMA
    {
    public:
        GenericLedMatrixIF(CRGB * crgb);
        
        void clear();
        void show();
        int XY(int16_t x, int16_t y); // compat with FastLED code, returns 1D offset

        static uint16_t Color(uint8_t r, uint8_t g, uint8_t b);

        virtual void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);

        virtual void drawPixel(int16_t x, int16_t y, uint16_t color); // overwrite adafruit implementation
        virtual void drawPixel(int16_t x, int16_t y, uint32_t color); // overwrite adafruit implementation
        virtual void drawPixel(int16_t x, int16_t y, const CRGB& crgb); // overwrite adafruit implementation

        virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);

    #ifndef NO_FAST_FUNCTIONS
        virtual inline void drawFastVLine(int16_t x, int16_t y, int16_t h, uint8_t r, uint8_t g, uint8_t b);
        virtual inline void drawFastHLine(int16_t x, int16_t y, int16_t w, uint8_t r, uint8_t g, uint8_t b);
    #endif
    protected:
        CRGB *p_crgb;
    };

#else
    #define MATRIX_WIDTH 32
    #define MATRIX_HEIGHT 8

    #include <FastLED_NeoMatrix.h>

    class GenericLedMatrixIF : public FastLED_NeoMatrix
    {
    public:
        GenericLedMatrixIF(CRGB * crgb, uint16_t matrixW, uint16_t matrixH, uint8_t tX, uint8_t tY
                            , uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS +
                            NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS) 
                            : FastLED_NeoMatrix(crgb, matrixW, matrixH, tX, tY, matrixType)
        {}  
        
        GenericLedMatrixIF(CRGB *leds, uint16_t w, uint16_t h, uint8_t matrixType = NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS +
                         NEO_TILE_TOP + NEO_TILE_LEFT + NEO_TILE_ROWS) 
                         : FastLED_NeoMatrix(leds, w, h, matrixType)
        {}
    };

#endif

#endif