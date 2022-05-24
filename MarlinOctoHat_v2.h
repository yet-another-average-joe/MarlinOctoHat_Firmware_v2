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

    ST9270      NSS : active HIGH : pages

                                            STM32F103C6 BluePill
                                                ----------
                                   NSS2  PB12 |*           | GND
                                   SCK2  PB13 |*           | GND
                                         PB14 |*           | 3V3
                                  MOSI2  PB15 |*           | RESET RESET
                                          PA8 |*          *| PB11
                                          PA9 |*          *| PB10
                                         PA10 |*           | PB1   PIN_DISPLAY_TYPE
                                         PA11 |*           | PB0
                                         PA12 |*           | PA7  MOSI1 GPIO20
                                         PA15 |*           | PA6  MISO1 GPIO19
                                         PB3  |*           | PA5  SCK1  GPIO21
                                         PB4  |*           | PA4  NSS1  GPIO16
                                         PB5  |*           | PA3
                     GPIO5  -   PIN_DTR  PB6  |*           | PA2  (RESERVED)
                    (GPIO17 - RESERVED)  PB7  |*           | PA1  (RESERVED)
                     (GPIO3 - RESERVED)  PB8  |*           | PA0  (RESERVED)
                     (GPIO2 - RESERVED)  PB9  |*           | PC15
                                          5V  |            | PC14
                                         GND  |            | PC13
                                         3V3  |            | VBAT
                                              |            |
                                                ----------
                                          * : 5V tolerant pins 
    
    
    LCD_E, LCD_4, LCD_7 : variant with ST7920 emulator
    SPI2 : SPI "input" (MOSI), from printer motherboard
    -------------------------------------------------- -

    STM32 Pins : 5V tolerant

    MoBo        OLED          BluePill
    XP1-5        CS        PB12    PIN_SS_2
    XP2-9        SCL       PB13    PIN_SCK_2
    N/C          PB14
    EXP2-5       SDA       PB15    PIN_MOSI_2

    SPI1 : SPI "output" (MISO), to RasPi
    ------------------------------------ -

    STM32 Pins : / !\ NOT 5V TOLERANT / !\

    PIN_RTS : high for READY_PULSE_DURATION ms when a new bitmap is available

    BluePill                            PasPi
    PA7    PIN_MOSI1    GPIO20    MOSI1
    PA6    PIN_MISO1    GPIO19    MISO1 
    PA5    PIN_SCK1     GPIO21    SCK1
    PA4    PIN_NSS1     GPIO16    NSS1
    PB6    PIN_DTR      GPIO5     data ready to send

    RESERVED PINS :
    
    (PB7)               GPIO17
    (PB8)               GPIO3 
    (PB9)               GPIO2
    (PA0)               GPIO1
    (PA1)               GPIO12
    (PA2)               GPIO6
*/

#include <SPI.h>

////////////////////////////////////////////////////////////////////////////////////////////
// SPI 2 : input ; DMA1, DMA_CH4

extern SPIClass SPI_2;  // definition in MarlinOctoHat.cpp

//  Emulators : graphics mode only ; text and mixed modes not implemented
//  WARNING : ST7920 : very first page (half frame/screen) will be "eaten" while sync at boot ; expected behaviour !

void ST7920_setup_SPI_2();  // ST7920 (RepRapDiscount Full Graphic Smart Controller)
void SSD1306_setup_SPI_2(); // SSD1306 / SSD1309 / SSH1106 OLED

// input buffer ; shared by all emulators ; size set to max display buffer of the two emulators (1280 bytes)
extern volatile uint8_t SPI_2_Rx_Buffer[];
extern volatile bool dataReady; // flag : common to the 2 emulators : DMA transfer complete

////////////////////////////////////////////////////////////////////////////////////////////
// SPI 1 = output ; DMA1, DMA_CH3

void setup_SPI_1();

// output buffer
#define BMP_OUT_WIDTH   128    // pixels
#define BMP_OUT_HEIGHT   64    // pixels
#define BMP_LINE_BYTES (BMP_OUT_WIDTH / 8)                      // 1 bit/pixel : 1024 bytes
#define BMP_OUT_SIZE   (BMP_OUT_WIDTH * BMP_OUT_HEIGHT / 8)     // 1 bit/pixel : 1024 bytes
extern volatile uint8_t bmpOut[BMP_OUT_SIZE]; // SPI 1 (output) bufffer

////////////////////////////////////////////////////////////////////////////////////////////
// debugging funtions

void printBmpHex();     // prints screen capture as hex dump on Serial
void printBmpAscii();   // prints screen capture as ASCII art on Serial
