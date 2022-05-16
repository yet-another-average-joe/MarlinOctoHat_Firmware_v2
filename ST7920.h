#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////
//  SPI Slave Sniffer for the ST7920
//  tested Ok with SPI SSD1309 OLED and Marlin v2.0.x on genuine and fake STM32F103C6("BluePill")
//  grpahics mode only ; text and mixed modes not implemented
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "MarlinOctoHat_v2.h"


/////////////////////////////////////////////////////////////////////////////////////////////////
// ST7920 SPI data description
// see ST7920 for an example

struct ST7920_lastLine_t // last line is one byte shorter than others
{
    // 7 bytes
    uint8_t prefix0[2]; // = { 0x30, 0xE0 }
    uint8_t prefix1[2]; // = { 0xNN, 0xMM } -> #line
    uint8_t prefix2;    // = 0x80
    uint8_t prefix3;    // page0 : 0x00, page1 : 0x80
    uint8_t prefix4;    // = 0xFA

    // 32 bytes
    uint8_t payload[32];    // payload ("encoded")

    // DOES NOT EXIST IN LAST LINE
    //uint8_t postfix;
};

struct ST7920_line_t : ST7920_lastLine_t // line : one byte more than last line
{
    // 1 byte
    uint8_t postfix;        // page0 last byte = 0x10, page1 last byte = 0x00 
};

struct ST7920_page_t // page ; a frame is made of two pages
{
    uint8_t             byte0;      // = 0xF8
    ST7920_line_t       lines[31];
    ST7920_lastLine_t   lastLine;
};

#define ST7920_LINE_SIZE        (sizeof(ST7920_line_t) / sizeof(uint8_t))              //   40 bytes
#define ST7920_PAYLOAD_SIZE     (sizeof(ST7920_line_t::payload) / sizeof(uint8_t))     //   32 bytes
#define ST7920_SPI_PAGE_SIZE    (sizeof(ST7920_page_t) / sizeof(uint8_t))              // 1280 bytes

#define ST7920_BUF_SIZE ST7920_SPI_PAGE_SIZE

void ST7920_setup_SPI_2();
void ST7920_setup_SPI_2_DMA();
void ST7920_ISR_NSS_2();
int32_t ST7920_dataToBmpOut();

//extern bool flagEndPage1;



