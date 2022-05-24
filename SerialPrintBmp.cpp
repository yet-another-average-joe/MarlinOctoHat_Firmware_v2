/*
 Name:       SerialPrintBmp.cpp
 Created:    2022/05/08
 Author:     Y@@J
 */

#include "MarlinOctoHat_v2.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prints screen capture as ASCII art to Serial

#ifdef __PRINT_ASCII

void printBmpAscii()
{
    Serial.println();

    volatile uint8_t* p = bmpOut;

    for (int iLine = 0; iLine < BMP_OUT_HEIGHT; iLine++)
    {
        for (int jCol = 0; jCol < BMP_LINE_BYTES; jCol++)
        {
            uint8_t b = *p++;
            int n = 7;

            while (n >= 0)
            {
                if (b & 1 << n)
                    Serial.print('#');
                else
                    Serial.print(' ');

                n--;
            }
        }
        Serial.println();
    }

    for (int x = 0; x < BMP_OUT_WIDTH; x++)
        Serial.print('-');
}

#endif //  __PRINT_ASCII

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prints screen capture as hex dump to Serial

#ifdef __PRINT_HEX

void printBmpHex()
{
    Serial.println();

    volatile uint8_t* p = bmpOut;

    for (int iLine = 0; iLine < BMP_OUT_HEIGHT; iLine++)
    {
        for (int jCol = 0; jCol < BMP_LINE_BYTES; jCol++)
        {
            uint8_t b = *p++;
            if (b < 0x10)
                Serial.print("0x0");
            else
                Serial.print("0x");
            Serial.print(b, HEX);
            Serial.print(", ");
        }
        Serial.println();
    }

    for (int x = 0; x < BMP_OUT_WIDTH; x++)
        Serial.print('-');
}

#endif // __PRINT_HEX

// END
