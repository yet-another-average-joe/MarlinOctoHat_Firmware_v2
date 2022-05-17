/*
 Name:       SSD1306.cpp
 Created:    2022/05/08
 Author:     Y@@J
 */

#include "SSD1306.h"
#include "SpiOut.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OLED    data

// data (SSD1306/SSD1309) : 1 frame = 8 pages, 1 page = 3 command bytes + 128 bytes, 1 page = 8 graphic lines
#define SSD1306_PAGE_COUNT      8
#define SSD1306_LINES_PER_PAGE  8
#define SSD1306_CMD_SIZE        3
#define SSD1306_PAGE_SIZE       128
#define SSD1306_SUB_FRAME_SIZE  (SSD1306_CMD_SIZE + SSD1306_PAGE_SIZE)          // = 131 bytes
#define SSD1306_FRAME_SIZE      (SSD1306_SUB_FRAME_SIZE * SSD1306_PAGE_COUNT)   // = 1048 bytes

// bmpOled with vertical 8bit monochrome macropixels...
uint8_t bmpOled[SSD1306_PAGE_COUNT][SSD1306_PAGE_SIZE] = {0};

// current page (just received)
//volatile uint32_t SSD1306_numPage = 0;

void SSD1306_setup_SPI_2_DMA();
void SSD1306_SPI_2_DMA_IRQ();
void SSD1306_PageToBmpOut(uint32_t numPage);

static volatile bool SSD1306_dataReady = false;

bool SSD1306_readyToSend()
{
    return SSD1306_dataReady;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : setup SPI_2 ; slave ; Rx only ; input

void SSD1306_setup_SPI_2()
{
    SPI_2.setModule(2); // STM32F103C8 : low density 1 or 2
    SPISettings spiSettings(0, MSBFIRST, SPI_MODE0, DATA_SIZE_8BIT);  // 0 Hz : set by master
    SPI_2.beginTransactionSlave(spiSettings);
    spi_rx_reg(SPI_2.dev()); // Clear Rx register in case we already received SPI data
    SSD1306_setup_SPI_2_DMA();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DMA tube for SPI2 Rx

dma_tube_config SSD1306_SPI_2_DMA_RxTubeCfg =
{
    &SPI2->regs->DR,            // Source of data
    DMA_SIZE_8BITS,             // Source transfer size
    &SPI_2_Rx_Buffer,           // Destination of data
    DMA_SIZE_8BITS,             // Destination transfer size
    SSD1306_SUB_FRAME_SIZE,     // Number of bytes to receive
                                // Flags :
    DMA_CFG_DST_INC |           //    - auto increment destination address
    DMA_CFG_CIRC |              //    - circular buffer
    DMA_CFG_CMPLT_IE |          //    - set tube full IRQ
    DMA_CFG_ERR_IE |            //    - want messages in the DMA irq
    DMA_CCR_PL_VERY_HIGH,       //    - very high priority
    0,                          //    - reserved
    DMA_REQ_SRC_SPI2_RX         //    - Hardware DMA request source
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : setup SPI_2 DMA ; Rx (MOSI) only ; input

void SSD1306_setup_SPI_2_DMA()
{
    dma_init(DMA1);

    int ret_rx = dma_tube_cfg(DMA1, SPI_2_RX_DMA_CH, &SSD1306_SPI_2_DMA_RxTubeCfg);

    if (ret_rx != DMA_TUBE_CFG_SUCCESS)
    {
        // if the tube creation failed, no hope the code will work !
        while (1)
        {
#ifdef __SERIAL_DEBUG
            Serial.print("setup SPI_2 Rx DMA configuration error: ");
            Serial.println(ret_rx, HEX);
            Serial.println("Reset is needed!");
            delay(100);
#else
            // hope it will trigger the watchdog...
#endif
        }
    }

    spi_rx_reg(SPI_2.dev()); // Clear RX register in case we already received SPI data
    dma_attach_interrupt(DMA1, SPI_2_RX_DMA_CH, SSD1306_SPI_2_DMA_IRQ); // Attach interrupt to catch end of DMA transfer
    dma_enable(DMA1, SPI_2_RX_DMA_CH); // Rx : Enable DMA configurations
    spi_rx_dma_enable(SPI_2.dev()); // SPI DMA requests for Rx 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : decode + copy data to bmpOut on NSS interrupt
// 270 microseconds

inline void SSD1306_PageToBmpOut(uint32_t numPage)
{
    uint8_t* pOut = bmpOut + numPage * SSD1306_PAGE_SIZE;

    memset((void*)(pOut), 0, SSD1306_PAGE_SIZE); // reset to zero ; 1024 bytes

    uint32_t iByte = 0; // bytes counter for output bitmap
    uint32_t jBit = 0; // bits counter for the current byte
    uint8_t* oled = ((uint8_t*)SPI_2_Rx_Buffer + 3); // raw data source

    for (uint32_t iLine = 0; iLine < SSD1306_LINES_PER_PAGE; iLine++) // 8 lines
        for (uint32_t jCol = 0; jCol < SSD1306_PAGE_SIZE; jCol++) // 128 pixels / line
        {
            // vertical byte to horizontal byte
            uint8_t x = *(oled + jCol);
            uint8_t y = 1 << iLine;
            if (x & y)
                pOut[iByte] |= (1 << (7 - jBit));

            jBit++;
            jBit %= 8;

            if (jBit == 0) // byte complete
                iByte++; // next byte
        }
}

void SSD1306_resetDMA()
{
    dma_disable(DMA1, SPI_2_RX_DMA_CH);
    dma_tube_cfg(DMA1, SPI_2_RX_DMA_CH, &SSD1306_SPI_2_DMA_RxTubeCfg);
    dma_enable(DMA1, SPI_2_RX_DMA_CH);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : SPI_2 Rx IRQ ; input

//#define __SERIAL_DEBUG
#undef __SERIAL_DEBUG

void SSD1306_SPI_2_DMA_IRQ()
{
    dma_irq_cause cause = dma_get_irq_cause(DMA1, SPI_2_RX_DMA_CH);

    if (cause == DMA_TRANSFER_COMPLETE)
    {
        SPI_2_DMA_transfer_complete = true;

        volatile uint8_t* p = SPI_2_Rx_Buffer;
        uint8_t numPage = *(p + 2) & 0x0F;

        // keep pages only : 0x10 0x00 0xBn (0 <= n < 8)
        //if ((*p++ != 0x10) || (*p++ != 0x00) || ((*p++ & 0xF0) != 0xB0) || ((*p & 0x0F) > 7)) // not a page or not in sync
        //if ((*p++ != 0x10) || (*p++ != 0x00))// || ((*p++ & 0xF0) != 0xB0))// || ((*p & 0x0F) > 7)) // not a page or not in sync
        if(numPage > 7)
        {
            //Serial.println(numPage);
            //SSD1306_resetDMA();
            nvic_sys_reset(); // C l'bordel ! (C)2015 Carlito, Safaga, diving resort

        }
        else
        {
            // page received : copy to bmpOut
            SSD1306_PageToBmpOut(numPage); // 270 microseconds

            // 8th page : data ready
            if (numPage == 7)
                SSD1306_dataReady = true;
            else
                SSD1306_dataReady = false;
        }
    }
    //else
    //{
    //    SSD1306_resetDMA();
    //}

    dma_clear_isr_bits(DMA1, SPI_2_RX_DMA_CH);
}
