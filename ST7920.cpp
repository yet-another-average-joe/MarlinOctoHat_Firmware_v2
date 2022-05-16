

#include "ST7920.h"
#include "SpiOut.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ST7920    SPI tranfers retro engineering (crappy datasheet !)
// data as seen on the logic analyser

/*
                typical frame, made of 2 pages of 32 lines :
                --------------------------------------------

                NSS active HIGH

                F8

                40 microseconds delay

                30 / E0 / 80 / 00 / 80 / 00 / FA / payload : 32 bytes / F8 : TOTAL = 40 bytes
                30 / E0 / 80 / 10 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / 20 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / 30 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / 40 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / 50 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / 60 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / 70 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / 80 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / 90 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / A0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / B0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / C0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / D0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / E0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 80 / F0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 00 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 10 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 20 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 30 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 40 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 50 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 60 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 70 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 80 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / 90 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / A0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / B0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / C0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / D0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / E0 / 80 / 00 / FA / payload : 32 bytes / F8
                30 / E0 / 90 / F0 / 80 / 00 / FA / payload : 31 bytes /    10 (page 0) last line = 39 bytes

                NSS inactive LOW

                560 microseconds
                
                NSS active HIGH

                F8
                
                40 us

                30 / E0 / nn / mm / 80 / 00 / FA

                30 / E0 / 80 / 00 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / 10 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / 20 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / 30 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / 40 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / 50 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / 60 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / 70 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / 80 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / 90 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / A0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / B0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / C0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / D0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / E0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 80 / F0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 00 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 10 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 20 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 30 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 40 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 50 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 60 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 70 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 80 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / 90 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / A0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / B0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / C0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / D0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / E0 / 80 / 80 / FA(7 bytes) / payload : 32 bytes / F8
                30 / E0 / 90 / F0 / 80 / 80 / FA(7 bytes) / payload : 31 bytes /     00 (page 1) last line = 39 bytes

                NSS inactive LOW
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : setup SPI_2 ; slave ; input

void ST7920_setup_SPI_2()
{
    SPI_2.setModule(2); // STM32F103C8 : low density 1 or 2
    SPISettings spiSettings(0, MSBFIRST, SPI_MODE0, DATA_SIZE_8BIT /* | SPI_SOFT_SS | SPI_SW_SLAVE*/);  // 0 Hz : set by master
    SPI_2.beginTransactionSlave(spiSettings);
    spi_rx_reg(SPI_2.dev()); // Clear Rx register in case we already received SPI data
    ST7920_setup_SPI_2_DMA();
    attachInterrupt(digitalPinToInterrupt(PIN_NSS_2), ST7920_ISR_NSS_2, RISING);   // FALLING = end of page (= 1/2 screen)
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// tube settings
dma_tube_config ST7920_SPI_2_DMA_RxTubeCfg =
{
    &SPI2->regs->DR,        // Source of data
    DMA_SIZE_8BITS,         // Source transfer size
    &SPI_2_Rx_Buffer,       // Destination of data
    DMA_SIZE_8BITS,         // Destination transfer size
    ST7920_SPI_PAGE_SIZE,   // Number of bytes to receive
                            // Flags :
    DMA_CFG_DST_INC |       //    - auto increment destination address
    DMA_CFG_CIRC |          //    - circular buffer
    DMA_CFG_CMPLT_IE |      //    - set tube full IRQ
    DMA_CFG_ERR_IE |        //    - want messages in the DMA irq
    DMA_CCR_PL_VERY_HIGH,   //    - very high priority
    0,                      //    - reserved
    DMA_REQ_SRC_SPI2_RX     //    - Hardware DMA request source
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : setup SPI_2 DMA

//#define __SERIAL_DEBUG

void ST7920_setup_SPI_2_DMA()
{
    dma_init(DMA1);

    int ret_rx = dma_tube_cfg(DMA1, SPI_2_RX_DMA_CH, &ST7920_SPI_2_DMA_RxTubeCfg);

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
    dma_attach_interrupt(DMA1, SPI_2_RX_DMA_CH, SPI_2_DMA_IRQ); // Attach interrupt to catch end of DMA transfer
    dma_enable(DMA1, SPI_2_RX_DMA_CH); // Rx : Enable DMA configurations
    spi_rx_dma_enable(SPI_2.dev()); // SPI DMA requests for Rx 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper function : returns the page # for the current data in SPI_2_Rx_Buffer
// error : returns -1 

static inline int32_t ST7920_getNumPage()
{
    // page 0 / page 1 hashcodes : faster
    static const uint8_t hash0 = 0xF8 ^ 0x30 ^ 0xE0 ^ 0x80 ^ 0x00 ^ 0x80 ^ 0x00 ^ 0xFA; // = 0xD2
    static const uint8_t hash1 = 0xF8 ^ 0x30 ^ 0xE0 ^ 0x80 ^ 0x00 ^ 0x80 ^ 0x80 ^ 0xFA; // = 0x52

    uint8_t hash = 0;

    uint8_t* p = (uint8_t*)SPI_2_Rx_Buffer;

    for (size_t i = 0; i < 8; i++)
        hash ^= p[i];

    switch (hash)
    {
    case hash0:
        return 0;

    case hash1:
        return 1;

    default:
        return -1;
    };
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ISR : NSS_2
// FALLING  (= end of page = 1/2 screen) : if not a payload frame, reset SPI + DMA

void ST7920_ISR_NSS_2()
{
    // keep valid pages only : reset
    if (ST7920_getNumPage() < 0)
    {
        dma_disable(DMA1, SPI_2_RX_DMA_CH);
        dma_tube_cfg(DMA1, SPI_2_RX_DMA_CH, &ST7920_SPI_2_DMA_RxTubeCfg);
        dma_enable(DMA1, SPI_2_RX_DMA_CH);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper function : decodes one line of payload

static inline void decodePayload(uint8_t* dst, uint8_t* src)
{
    for (size_t i = 0; i < 32; i += 2)
    {
        uint8_t a = *src++;
        uint8_t b = *src++;
        *dst++ = a | ((b >> 4) & 0x0F);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : converts the ST7920 bitmap into bmpOut bitmap

int32_t ST7920_dataToBmpOut()
{
    uint8_t* bmpPtr = bmpOut;
    int32_t numPage = ST7920_getNumPage();

    if (numPage == 1)
        bmpPtr += BMP_OUT_SIZE / 2; // fill 2nd half of the bitmap
    else if (numPage == -1)
        return -1; // ERROR

    for (size_t i = 0; i < 32; i++)
    {
        ST7920_page_t* pPage = (ST7920_page_t*)SPI_2_Rx_Buffer;
        ST7920_line_t* pLine = &pPage->lines[i];
        decodePayload((uint8_t*)bmpPtr, pLine->payload);
        bmpPtr += BMP_LINE_BYTES * sizeof(uint8_t);
    }

    return numPage;
}