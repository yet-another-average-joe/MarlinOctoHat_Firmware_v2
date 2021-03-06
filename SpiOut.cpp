/*
 Name:       SpiOut.cpp
 Created:    2022/05/08
 Author:     Y@@J
 */

#include "MarlinOctoHat_v2.h"

// receives the decoded bitmap
// will be used as Tx buffer for SPI_1

volatile uint8_t bmpOut[BMP_OUT_SIZE] = {0}; // BMP_OUT_SIZE = 1024 bytes

void setup_SPI_1_DMA();

#define SPI_1   SPI // SPI 1, DMA1, DMA_CH3

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : setup SPI_1 ; slave, Rx only

void setup_SPI_1()
{
    SPI_1.setModule(1); // STM32F103C8 : low density 1 or 2
    SPISettings spiSettings(0, MSBFIRST, SPI_MODE0, DATA_SIZE_8BIT); // 0 Hz : set by master
    SPI_1.beginTransactionSlave(spiSettings);
    setup_SPI_1_DMA();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : setup SPI_1 DMA ; Tx (MISO) only

void setup_SPI_1_DMA()
{
    dma_init(DMA1);

    // DMA tube configuration for SPI1 Tx ; data = bmpOut
    dma_tube_config SPI_1_DMA_TxTubeCfg =
    {
        bmpOut,                     // Data source
        DMA_SIZE_8BITS,             // Source transfer size
        &SPI1->regs->DR,            // Destination of data
        DMA_SIZE_8BITS,             // Destination transfer size
        BMP_OUT_SIZE,             // Number of bytes to transfer
                                    // Flags :
        DMA_CFG_SRC_INC |           // auto increment source address
        DMA_CFG_CIRC |              // circular buffer
        DMA_CFG_CMPLT_IE |          // set tube full IRQ
        DMA_CFG_ERR_IE |            // want messages in the DMA irq
        DMA_CCR_PL_VERY_HIGH,       // very high priority
        0,                          // reserved
        DMA_REQ_SRC_SPI1_TX         // Hardware DMA request source
    };

    int ret_tx = dma_tube_cfg(DMA1, DMA_CH3, &SPI_1_DMA_TxTubeCfg);

    if (ret_tx != DMA_TUBE_CFG_SUCCESS)
    {
        // if the tube creation failed, no hope the code will work !
        while (1)
        {
#ifdef __SERIAL_DEBUG
            Serial.print("setup SPI_1 Tx DMA configuration error: ");
            Serial.println(ret_tx, HEX);
            Serial.println("Reset is needed!");
            delay(100);
#else
            // hope it will trigger the watchdog...
#endif
        }
    }

    dma_enable(DMA1, DMA_CH3); // Tx : Enable DMA configurations
    spi_tx_dma_enable(SPI_1.dev()); // SPI DMA requests for Tx 
}

// END
