#pragma once
/*
 Name:       MarlinOctoHat_v2.h
 Created:    2022/05/08
 Author:     Y@@J
 */
/*
    thread on stm32duino.com, and special thanks for the help about SPI DMA tranfers !:
    https://www.stm32duino.com/viewtopic.php?t=825

    SSD1306 datasheet :
    https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

    SPI data :
    1 SPI frame = 8 pages
    1 page = 8lines / 128 pixels, vertical macropixels
    1 page : 3 command bytes + 128 bytes(= 8 lines on the display)
    1 command byte = 0x10 0x00 0XBn, n = page # (0...7)
    total : 8 pages; 8 * 131 = 1048 bytes, CLK @ 1MHz

    /////////////////////////////////////////////////////////////////////////////////////
    // OLED wirings

    SSD1306-9          SPI
    SH1106
    SCL                SCLK
    SDA                MOSI
    RES                N / A
    DC                 N / A
    SS                 SS

    SS : LOW for pages, including commands; HIGH between pages and between frames
    DC : LOW for commands only
    // BluePill / RaspBerry Pi wirings

                                                ----------
                     (LCD_5)   [NSS2]    PB12 |            | GND
                       (SCK)   [SCK2]    PB13 |            | GND
                                         PB14 |            | 3V3
                      (MOSI)  [MOSI2]    PB15 |            | RESET RESET
                                          PA8 |            | PB11
                                          PA9 |            | PB10
                                         PA10 |            | PB1   PIN_DISPLAY_TYPE
                                         PA11 |            | PB0
                                         PA12 |            | PA7   [MOSI1]     -> GPIO20(SPI1 MOSI)
                                         PA15 |            | PA6   [MISO1]     -> GPIO19(SPI1 MISO)
                                         PB3  |            | PA5   [SCK1]      -> GPIO21(SPI1 SCLK)
                                         PB4  |            | PA4   [NSS1]      -> GPIO16(SPI1 CS0)
                                         PB5  |            | PA3
          GPIO5 < -DATA_DTR              PB6  |            | PA2   (PIN_RELAY_OFF)
        (GPIO17 < -PIN_SOFT_SHUTDOWN)    PB7  |            | PA1   (PIN_RELAY_ON)
         (GPIO3 < -PIN_HARD_SHUTDOWN)    PB8  |            | PA0   (PIN_BTN_POWER)
         (GPIO2 < -PIN_POWEROFF)         PB9  |            | PC15
                                          5V  |            | PC14
                                         GND  |            | PC13
                                         3V3  |            | VBAT
                                              |            |
                                                ----------
    LCD_E, LCD_4, LCD_7 : variant with ST7920 emulator(not implemented)
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
    PA7    PIN_MOSI1[38]            GPIO20    MOSI1
   (PA6    PIN_MISO1[35]            GPIO19    MISO1)
    PA5    PIN_SCK1[40]             GPIO21    SCK1
    PA4    PIN_NSS1[36]             GPIO16    NSS1
    PB6    PIN_RTS[29]              GPIO5     data ready to send
    PB7    PIN_SOFT_SHUTDOWN[11]    GPIO17    output, default = LOW, HIGH = short press->OctoPrint Shutdown
    PB8    PIN_HARD_SHUTDOWN[5]     GPIO3     output, default = LOW, HIGH triggered by PIN_BTN_POWER long press->OS Shutdown
    PB9    PIN_POWEROFF[3]          GPIO2     input, interrupt
    PA0    PIN_BTN_POWER[33]        GPIO13    input, pullup, interrupt, sends 1 sec LOW pulse to PIN_HARD_SHUTDOWN
    PA1    PIN_RELAY_ON[32]         GPIO12    output, default = LOW, HIGH for 1 sec at boot
    PA2    PIN_RELAY_OFF[31]        GPIO6     output, default = LOW, HIGH triggerd by PIN_POWEROFF
*/
#include <SPI.h>

#define PIN_RTS             PB6 // Ready To Send
#define PIN_DISPLAY_TYPE    PB1 // LOW : SSD1606, HIGH : ST7920
#define PIN_NSS_2           PB12

#define BMP_OUT_WIDTH   128    // pixels
#define BMP_OUT_HEIGHT   64    // pixels

#define BMP_LINE_BYTES (BMP_OUT_WIDTH / 8)                      // 1 bit/pixel : 1024 bytes
#define BMP_OUT_SIZE   (BMP_OUT_WIDTH * BMP_OUT_HEIGHT / 8)     // 1 bit/pixel : 1024 bytes

//PIN_SOFT_SHUTDOWN
//PIN_HARD_SHUTDOWN
//PIN_POWEROFF
//PIN_RELAY_OFF
//PIN_RELAY_ON
//PIN_BTN_POWER

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SPI 2 : input

#define SPI_2_RX_DMA_CH        DMA_CH4	// input
extern SPIClass SPI_2;                      // declared in MarlinOctoHat.cpp

extern volatile uint8_t SPI_2_Rx_Buffer[];

extern volatile bool SPI_2_DMA_transfer_complete;
extern volatile bool SPI_2_DMA_error;
