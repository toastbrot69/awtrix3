#ifndef LEDMATRIX_H
#define LEDMATRIX_H

/*
class GenericLedMatrixIF
{
public:
    virtual ~GenericLedMatrixIF() {}
};
*/
#ifdef USE_HUB75
    #define MATRIX_WIDTH 64 
    #define MATRIX_HEIGHT 32

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

    #include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

    class GenericLedMatrixIF : public MatrixPanel_I2S_DMA
    {
    public:
        GenericLedMatrixIF(const HUB75_I2S_CFG& mxconfig) : MatrixPanel_I2S_DMA(mxconfig)
        {}  
        
        void clear();
        void show();
        int XY(int16_t x, int16_t y); // compat with FastLED code, returns 1D offset

        static uint16_t Color(uint8_t r, uint8_t g, uint8_t b);

        void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
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
    };

#endif

#endif