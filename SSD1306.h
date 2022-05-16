#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////
//  SPI Slave Sniffer for the SSD1306 / SSD1309 / SSH1106 OLED
//  tested Ok with SPI SSD1309 OLEDand Marlin v2.0.x on genuine and fake STM32F103C6("BluePill")
//  graphics mode only, text modee not implemented
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "MarlinOctoHat_v2.h"

// data (SSD1306/SSD1309) : 1 frame = 8 pages, 1 page = 3 command bytes + 128 bytes, 1 page = 8 graphic lines
#define SSD1306_PAGE_COUNT      8
#define SSD1306_LINES_PER_PAGE  8
#define SSD1306_CMD_SIZE        3
#define SSD1306_PAGE_SIZE       128
#define SSD1306_SPI_PAGE_SIZE   (SSD1306_CMD_SIZE + SSD1306_PAGE_SIZE)        // 1048 bytes
#define SSD1306_SPI_FRAME_SIZE  (SSD1306_PAGE_COUNT * SSD1306_SPI_PAGE_SIZE)  // 1024 bytes

#define SSD1306_BUF_SIZE SSD1306_SPI_PAGE_SIZE

void SSD1306_setup_SPI_2();
void SSD1306_setup_SPI_2_DMA();
void SSD1306_ISR_NSS_2();

bool SSD1306_stripCmdBytes();
void SSD1306_dataToBmpOut();



