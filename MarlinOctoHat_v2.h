#pragma once
/*
 Name:       MarlinOctoHat_v2.h
 Created:    2022/05/08
 Author:     Y@@J
 */

/*
    thread on stm32duino.com, and special thanks for the help about SPI DMA tranfers !:
    https://www.stm32duino.com/viewtopic.php?t=825

    SSD1306     NSS : active LOW : pages, including commands
                DC  : active LOW : commands only ; not used

    ST9270      NSS : /!\ active HIGH /!\ : pages


                                            STM32F103C6 BluePill
                                                ----------
                                   NSS2  PB12 |*           | GND
                                   SCK2  PB13 |*           | GND
                                     NC  PB14 |*           | 3V3
                                  MOSI2  PB15 |*           | RESET RESET
                                          PA8 |*          *| PB11
                                          PA9 |*          *| PB10
                                         PA10 |*           | PB1  PIN_DISPLAY_TYPE
                                         PA11 |*           | PB0
                                         PA12 |*           | PA7  MOSI1 - GPIO20
                                         PA15 |*           | PA6  MISO1 - GPIO19
                                         PB3  |*           | PA5  SCK1  - GPIO21
                                         PB4  |*           | PA4  NSS1  - GPIO16
                                         PB5  |*           | PA3
                      GPIO5  - PIN_DTR   PB6  |*           | PA2  (RESERVED)
                    (GPIO17 - RESERVED)  PB7  |*           | PA1  (RESERVED)
                     (GPIO3 - RESERVED)  PB8  |*           | PA0  (RESERVED)
                     (GPIO2 - RESERVED)  PB9  |*           | PC15
                                          5V  |            | PC14
                                         GND  |            | PC13
                                         3V3  |            | VBAT
                                              |            |
                                                ----------
                                          * : 5V tolerant pins 


    SPI1 : SPI "input", from EXP-1/EXP-2 to STM32
    =============================================

    Emulation mode set by jumpers on the hat

        STM32                   OLED*                   RRFGSC**
    PB12 / NSS2         CS  / EXP1-5 (LCD_5/CS)     LCD_RS /  EXP1-4 (RES)
    PB13 / SCK2         SCL / EXP2-2 (SCK)          LCD_4  /  EXP1-5
    PB15 / MOSI2        SDA / EXP2-6 (MOSI)         LCD_E  /  EXP1-3

    *  OLED SSD1306 / SSD1309 / SSH1106 (SPI mode ONLY)
    ** LCD ST7920 RepRap Discount Full Graphic Smart Controller
     
    All STM32 inputs are 5V tolerant for compatibility with older 5V printer motherboards


    SPI1 : SPI "output", to RasPi
    =============================

      BluePill                     PasPi
    PA7     MOSI1           GPIO20      MOSI1
    PA6     MISO1           GPIO19      MISO1 
    PA5     SCK1            GPIO21      SCK1
    PA4     NSS1            GPIO16      NSS1
    PB6     DTR             GPIO5       "Data Ready", high for DTR_PULSE_DURATION microseconds
                                                      when a new bitmap is available

    STM32 outputs : /!\ NOT 5V TOLERANT /!\


    RESERVED PINS (power management, optionnal, TODO) :
    ===================================================
    
    PB7               GPIO17
    PB8               GPIO3 
    PB9               GPIO2
    PA0               GPIO1
    PA1               GPIO12
    PA2               GPIO6
*/

#include <SPI.h>

////////////////////////////////////////////////////////////////////////////////////////////
// SPI 2 : input ; DMA1, DMA_CH4 ; default BluePill SPI 2 pins

extern SPIClass SPI_2;  // definition in MarlinOctoHat.cpp

//  Emulators : graphics mode only ; text and mixed modes not implemented
//  WARNING : ST7920 : very first page (half frame/screen) will be "eaten" by sync at boot
//                     this is the expected behaviour !

void ST7920_setup_SPI_2();  // ST7920 (RepRapDiscount Full Graphic Smart Controller)
void SSD1306_setup_SPI_2(); // SSD1306 / SSD1309 / SSH1106 OLED

// input buffer ; shared by all emulators ; size set to larger display buffer of the two emulators (1280 bytes)
extern volatile uint8_t SPI_2_Rx_Buffer[];
extern volatile bool dataReady; // flag : common to the 2 emulators : DMA transfer complete

////////////////////////////////////////////////////////////////////////////////////////////
// SPI 1 = output ; DMA1, DMA_CH3 ; default BluePill SPI 1 pins

void setup_SPI_1();

// output buffer
#define BMP_OUT_WIDTH           128                                     // pixels
#define BMP_OUT_HEIGHT           64                                     // pixels
#define BMP_LINE_BYTES          (BMP_OUT_WIDTH / 8)                     // 1 bit/pixel : 1024 bytes
#define BMP_OUT_SIZE            (BMP_OUT_WIDTH * BMP_OUT_HEIGHT / 8)    // 1 bit/pixel : 1024 bytes
extern volatile uint8_t         bmpOut[BMP_OUT_SIZE];                   // SPI 1 (output) bufffer

#define PIN_DTR                 PB6                                     // data ready for Raspberry Pi
#define PIN_DISPLAY_TYPE        PB1                                     // emulation : LOW : SSD1606, HIGH : ST7920 (jumper = JP3)

#define DTR_PULSE_DURATION      100                                     // microseconds


////////////////////////////////////////////////////////////////////////////////////////////
// debugging

//#define __SERIAL_DEBUG
//#define __PRINT_HEX   // prints bitmaps to Serial as ASCII Hex array 32x32
//#define __PRINT_ASCII // prints bitmaps to Serial as ASCII Art

#if defined(__PRINT_HEX) || defined(__PRINT_ASCII)
#define __SERIAL_DEBUG
#endif

#ifdef __SERIAL_DEBUG

#ifdef __PRINT_HEX
void printBmpHex();     // prints screen capture as hex dump to Serial
#endif // __PRINT_HEX

#ifdef __PRINT_ASCII
void printBmpAscii();   // prints screen capture as ASCII art to Serial
#endif // __PRINT_ASCII

#endif // __SERIAL_DEBUG

