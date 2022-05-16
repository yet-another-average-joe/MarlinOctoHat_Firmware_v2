#pragma once
/*
 Name:       ST7920.h
 Created:    2022/05/08
 Author:     Y@@J
 */

/////////////////////////////////////////////////////////////////////////////////////////////////
//  SPI Sniffer for the ST7920
//  tested Ok with RepRapDiscount Full Graphic Smart Controller and Marlin v2.0.x
//  on genuine and fake STM32F103C6("BluePill")
//  grpahics mode only ; text and mixed modes not implemented
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "MarlinOctoHat_v2.h"

void ST7920_setup_SPI_2();
int32_t ST7920_dataToBmpOut();

