/*
 Name:       MarlinOctoHat_v2.ino
 Created:    2022/05/08
 Author:     Y@@J
 */

#define __SERIAL_DEBUG
//#define __PRINT_HEX   // prints bitmaps to Serial as ASCII Hex array 32x32
//#define __PRINT_ASCII // prints bitmaps to Serial as ASCII Art

#if defined(__PRINT_HEX) || defined(__PRINT_ASCII)
#define __SERIAL_DEBUG
#endif

// power management
//#define __POWER

#include <stdlib.h>
#include "MarlinOctoHat_v2.h"
#include "SSD1306.h"
#include "ST7920.h"
#include "PrintBmp.h"
#include "SpiOut.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI 2

SPIClass    SPI_2(2);   // SPI in
//void SPI_2_DMA_IRQ();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mode : SSD1306 or ST7920

enum modeEmu_t
{
    SSD1306,
    ST7920
};

modeEmu_t  modeEmu;

// raw data buffer ; set buffer size to largest :
// ST7920_BUF_SIZE     : 1280 ; 1 page = half screen 
// SSD1306_BUF_SIZE    : 131 : 3 control bytes + page

volatile uint8_t SPI_2_Rx_Buffer[3000];// 1280];

// flags
volatile bool SPI_2_DMA_transfer_complete = false;
volatile bool SPI_2_DMA_error = false;
volatile bool dataReady = false;

// helper : returns dataReady (set by SPI2 DMA IRQ) and resets the flag

bool getDataReady()
{
    if (dataReady)
    {
        dataReady = false;
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : debugPrint() : displays ASCII art or HEX dump

inline void debugPrint()
{
#ifdef __SERIAL_DEBUG
#ifdef __PRINT_HEX
    printBmpHex();
#endif//  __PRINT_HEX

#ifdef __PRINT_ASCII
    printBmpAscii();
#endif // __PRINT_ASCII
#endif // __SERIAL_DEBUG
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : "Arduino" setup()

void setup()
{
#ifdef __SERIAL_DEBUG
    Serial.begin(115200);
#endif

#ifdef __POWER
	setupPower();
#endif // __POWER

    pinMode(LED_BUILTIN, OUTPUT); // blinks when display is updated
    digitalWrite(LED_BUILTIN, HIGH);

    pinMode(PIN_RTS, OUTPUT);
    digitalWrite(PIN_RTS, LOW);
    
    pinMode(PIN_NSS_2, INPUT);    // set by SPI library ?

    // LOW : SSD1306, HIGH : ST7920
    pinMode(PIN_DISPLAY_TYPE, INPUT_PULLUP);

    if (digitalRead(PIN_DISPLAY_TYPE) == HIGH) // HIGH = ST7920
    {
        modeEmu = ST7920;
        ST7920_setup_SPI_2();
    }
    else // SSD1306
    {
        modeEmu = SSD1306;
        SSD1306_setup_SPI_2();
    }

    setup_SPI_1();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : "Arduino" loop()

void loop()
{
    if (getDataReady())
        pulseDTR();
}

// END


