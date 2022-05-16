/*
 Name:       MarlinOctoHat.ino
 Created:    2021/08/31 // 2022/05/08
 Author:     Y@@J
 */

/*
            MOD

    PA8 : OUTPUT
    PA9 : INPUT


    - prendre signal CS sur pin header
    - cabler sur PA9

    set IRQ PA9 CHANGE

    IRQ : digitalWrite(PA8, !digitalRead(PA9))

    cabler PA8 -> PB12


    registers
*/

#define __SERIAL_DEBUG
//#define __PRINT_HEX   // prints bitmaps to Serial as ASCII Hex array 32x32
#define __PRINT_ASCII // prints bitmaps to Serial as ASCII Art

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

SPIClass    SPI_2(2);       // SPI in
void SPI_2_DMA_IRQ();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mode : SSD1306 or ST7920

enum modeEmu_t
{
    SSD1306,
    ST7920
};

modeEmu_t  modeEmu;

// SPI in buffer ; receives 1 page = half screen 
// raw data buffer ; set buffer size to larger :
// ST7920_BUF_SIZE     : 1280
// SSD1306_BUF_SIZE    : 1048

uint8_t SPI_2_Rx_Buffer[1024 + 512];// ST7920_BUF_SIZE];

// flags
volatile bool SPI_2_DMA_transfer_complete = false;
volatile bool SPI_2_DMA_error = false;

void PA8_ISR()
{
    // propagation 2.8 microseconds ; must be less  than 1 us : 1 MHz
    //digitalWrite(PA9, !digitalRead(PA8));


    // 2.6us
    //bool b = digitalRead(PA8);
    //gpio_write_bit(GPIOA, 8, b);

    // 2.0 us
    //int b = (GPIOA->regs->IDR & (1 << 8)) << 1;

    //if(b)
    //    GPIOA->regs->ODR &= ~(1 << 9); // remove
    //else
    //    GPIOA->regs->ODR |= (1 << 9); // set

    gpio_toggle_bit(GPIOA, 9);
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

    //SPI_2.nssPin()

    pinMode(LED_BUILTIN, OUTPUT); // blinks when display is updated
    digitalWrite(LED_BUILTIN, HIGH);

    pinMode(PIN_RTS, OUTPUT);
    digitalWrite(PIN_RTS, LOW);

    // LOW : SSD1306, HIGH : ST7920
    pinMode(PIN_DISPLAY_TYPE, INPUT_PULLUP);
    
    pinMode(PIN_NSS_2, INPUT);    // set by SPI library
    pinMode(PA8, INPUT);
    pinMode(PA9, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(PA8), PA8_ISR, CHANGE);   // FALLING = end of page (= 1/2 screen)

    if (digitalRead(PIN_DISPLAY_TYPE) == HIGH) // HIGH = ST7920
    {
        ST7920_setup_SPI_2();
        //pinMode(PIN_NSS_2, INPUT_PULLDOWN);    // set by SPI library
    }
    else // SSD1306
        SSD1306_setup_SPI_2();

    setup_SPI_1();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : "Arduino" loop()

void loop()
{
    bool gotIt = false;

    if (SPI_2_DMA_transfer_complete)
    {
        if (modeEmu == SSD1306 && SSD1306_stripCmdBytes())
        {
            SSD1306_dataToBmpOut();
            pulseDTR();
            gotIt = true;
        }
        else // ST7920
        {
            if (ST7920_dataToBmpOut() == 1) // wait for the two pages
            {
                //ST7920_dataToBmpOut();
                pulseDTR();
                gotIt = true;
                //delay(10);
                printBmpAscii();
            }
        }

        
        SPI_2_DMA_transfer_complete = false; // reset flag
    }

#if defined(__PRINT_ASCII) || defined(__PRINT_HEX)
    if (gotIt)
    {
#ifdef __PRINT_HEX
        //if (modeEmu == SSD1306 || (modeEmu == ST7920 && flagEndPage1))
            printBmpHex();
#endif

#ifdef __PRINT_ASCII
        //if (modeEmu == SSD1306 || (modeEmu == ST7920 && flagEndPage1))
            //printBmpAscii();
#endif
    }
#endif

#ifdef __POWER
	loopPower();
#endif // __POWER

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : SPI_2 Rx IRQ ; input

#define __SERIAL_DEBUG
//#undef __SERIAL_DEBUG

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


