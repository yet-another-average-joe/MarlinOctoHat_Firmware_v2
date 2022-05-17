/*
 Name:       MarlinOctoHat_v2.ino
 Created:    2022/05/08
 Author:     Y@@J
 */

//#define __SERIAL_DEBUG
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
void SPI_2_DMA_IRQ();

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
// SSD1306_BUF_SIZE    : 1048 ; full screen

volatile uint8_t SPI_2_Rx_Buffer[1280];

// flags
volatile bool SPI_2_DMA_transfer_complete = false;
volatile bool SPI_2_DMA_error = false;

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
#ifdef __SERIAL_DEBUG
    bool gotIt = false;
#endif // __SERIAL_DEBUG

    if (SPI_2_DMA_transfer_complete)
    {
        if (modeEmu == SSD1306)
        {
            if (SSD1306_dataToBmpOut())
            {
                pulseDTR();
                debugPrint();
            }
#ifdef __SERIAL_DEBUG
            else
                Serial.println("loop() : SSD1306_dataToBmpOut() returned false"); // should never happen !
#endif // __SERIAL_DEBUG
        }
        else if (modeEmu == ST7920)
        {
            int numPage = ST7920_dataToBmpOut();

            if (numPage == 1) // wait for the two pages
            {
                pulseDTR();
                debugPrint();
            }
#ifdef __SERIAL_DEBUG
            else if (numPage == -1) // wait for the two pages
                Serial.println("loop() : ST7920_dataToBmpOut() returned numPage = -1"); // should never happen !
#endif // __SERIAL_DEBUG
        }

        SPI_2_DMA_transfer_complete = false; // reset flag
    }

#ifdef __POWER
	loopPower();
#endif // __POWER
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : SPI_2 Rx IRQ ; input

//#define __SERIAL_DEBUG
#undef __SERIAL_DEBUG

void SPI_2_DMA_IRQ()
{
    dma_irq_cause cause = dma_get_irq_cause(DMA1, SPI_2_RX_DMA_CH);

#ifdef __SERIAL_DEBUG
    switch (cause)
    {
    case DMA_TRANSFER_COMPLETE:            // Transfer is complete
        SPI_2_DMA_transfer_complete = true;
        SPI_2_DMA_error = false;
        Serial.println("DMA_2 : DMA_TRANSFER_COMPLETE");
        break;
    case DMA_TRANSFER_ERROR:            // Error occurred during transfer
        Serial.println("DMA_2 : DMA_TRANSFER_ERROR");
        SPI_2_DMA_error = true;
        break;
    case DMA_TRANSFER_DME_ERROR:        // Direct mode error occurred during transfer
        Serial.println("DMA_2 : DMA_TRANSFER_DME_ERROR");
        SPI_2_DMA_error = true;
        break;
    case DMA_TRANSFER_FIFO_ERROR:        // FIFO error occurred during transfer
        Serial.println("DMA_2 : DMA_TRANSFER_FIFO_ERROR");
        SPI_2_DMA_error = true;
        break;
    case DMA_TRANSFER_HALF_COMPLETE:    // Transfer is half complete
        Serial.println("DMA_2 : DMA_TRANSFER_HALF_COMPLETE");
        SPI_2_DMA_error = true;
        break;
    default:
        Serial.println("DMA_2 : UNKNOWN ERROR");
        SPI_2_DMA_error = true;
        break;
    }
#else
    if (cause == DMA_TRANSFER_COMPLETE)
    {
        SPI_2_DMA_transfer_complete = true;
        SPI_2_DMA_error = false;
    }
    else
        SPI_2_DMA_error = true;
#endif // __SERIAL_DEBUG

    dma_clear_isr_bits(DMA1, SPI_2_RX_DMA_CH);
}

// END


