

#include "SSD1306.h"
#include "SpiOut.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OLED    data

// bmpOled with vertical 8bit monochrome macropixels...
uint8_t bmpOled[SSD1306_PAGE_COUNT][SSD1306_PAGE_SIZE] = {0};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : setup SPI_2 ; slave ; Rx only ; input

void SSD1306_setup_SPI_2()
{
    SPI_2.setModule(2); // STM32F103C8 : low density 1 or 2
    SPISettings spiSettings(0, MSBFIRST, SPI_MODE0, DATA_SIZE_8BIT);  // 0 Hz : set by master
    SPI_2.beginTransactionSlave(spiSettings);
    spi_rx_reg(SPI_2.dev()); // Clear Rx register in case we already received SPI data
    SSD1306_setup_SPI_2_DMA();
    attachInterrupt(digitalPinToInterrupt(PIN_NSS_2), SSD1306_ISR_NSS_2, RISING);   // RISING = end of page (1/8 screen)
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DMA tube for SPI2 Rx

dma_tube_config SSD1306_SPI_2_DMA_RxTubeCfg =
{
    &SPI2->regs->DR,            // Source of data
    DMA_SIZE_8BITS,             // Source transfer size
    &SPI_2_Rx_Buffer,          // Destination of data
    DMA_SIZE_8BITS,             // Destination transfer size
    1048,//SSD1306_SPI_FRAME_SIZE,     // Number of bytes to receive
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
    dma_attach_interrupt(DMA1, SPI_2_RX_DMA_CH, SPI_2_DMA_IRQ); // Attach interrupt to catch end of DMA transfer
    dma_enable(DMA1, SPI_2_RX_DMA_CH); // Rx : Enable DMA configurations
    spi_rx_dma_enable(SPI_2.dev()); // SPI DMA requests for Rx 
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ISR : NSS_2
// RISING (= end of page = 1/8 screen): if not a payload frame, reset SPI + DMA
// pages begin with 0x10 0x00 0xB6 nn, 0 <= n <= 7)
// needed for resynch

void SSD1306_ISR_NSS_2()
{
	uint8_t* p = SPI_2_Rx_Buffer;

	// keep pages only : 0x10 0x00 0xBn (0 <= n < 8)
	if ((*p++ != 0x10) || (*p++ != 0x00) || ((*p++ & 0xF0) != 0xB0) || ((*p & 0x0F) > 7)) // not a page or not in sync
	{
		// reset
		dma_disable(DMA1, SPI_2_RX_DMA_CH);
		dma_tube_cfg(DMA1, SPI_2_RX_DMA_CH, &SSD1306_SPI_2_DMA_RxTubeCfg);
		dma_enable(DMA1, SPI_2_RX_DMA_CH);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : copies the SPI/DMA buffer to bmpOut, stripping the command bytes
// duration  30us

bool SSD1306_stripCmdBytes()
{
	for (size_t iPage = 0; iPage < SSD1306_PAGE_COUNT; iPage++)
	{
		uint8_t* pRxBuf = (uint8_t*)SPI_2_Rx_Buffer + iPage * SSD1306_SPI_PAGE_SIZE;
		uint16_t cmd = *(uint16_t*)pRxBuf; // command bytes
		uint8_t pageNum = *(uint8_t*)(pRxBuf + 2);

		// test 1st 3 bytes (= command bytes) of each frame ; 0x10 0x00 0xBn , n = frame #
		// shouldn't happen, see /CS ISR...
		if (cmd != 0x10 || pageNum != iPage + 0xB0)
		{
#ifdef __SERIAL_DEBUG
			Serial.println("stripOledCmdBytes() : ERROR");
#endif
			return false; // error, not in sync, etc.
		}

		// copy pages to bmpOled, drop command bytes
		memcpy((uint8_t*)&bmpOled + iPage * SSD1306_PAGE_SIZE, pRxBuf + SSD1306_CMD_SIZE, SSD1306_PAGE_SIZE);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : converts the OLED bitmap into bmpOut bitmap
//
//    SSD1306/SSD1309/SSH1106 : 8 pages with 128 8bit vertical macropixels
//    bmpOut : 1024 byes, 1bit linear bitmap (128 lines 64 columns)
//    Similar to a matrix transposition, each elementaty matrix being 8x8
//    duration : 2.80ms

void SSD1306_dataToBmpOut()
{
#ifdef __SERIAL_DEBUG
    uint32 t0 = micros();
#endif // __SERIAL_DEBUG

    uint16_t iByte = 0; // bytes counter for output bitmap
    uint16_t jBit = 0; // bits counter for the current byte
    memset((void*)bmpOut, 0, SSD1306_PAGE_COUNT * SSD1306_PAGE_SIZE); // reset to zero ; 1024 bytes

    for (uint8_t page = 0; page < SSD1306_PAGE_COUNT; page++)
        for (uint8_t line = 0; line < SSD1306_LINES_PER_PAGE; line++)
            for (uint8_t col = 0; col < SSD1306_PAGE_SIZE; col++)
            {
                // vertical byte to horizontal byte
                if (bmpOled[page][col] & (1 << line))
                    bmpOut[iByte] |= (1 << (7 - jBit));

                jBit++;
                jBit %= 8;

                if (jBit == 0) // byte complete
                    iByte++; // next byte
            }

#ifdef __SERIAL_DEBUG
    Serial.println(micros() - t0);
#endif // __SERIAL_DEBUG
}
