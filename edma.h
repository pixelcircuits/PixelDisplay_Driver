//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const edma_user_config_t edmaUserConfig = {
	.chnArbitration    = kEDMAChnArbitrationRoundrobin,
	.notHaltOnError    = false
};

//-----------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------
volatile edma_software_tcd_t TCDMem[4];
volatile edma_software_tcd_t* tcd0;
volatile edma_software_tcd_t* tcd1;
volatile edma_chn_state_t chnState;

//-----------------------------------------------------------------------
// Buffer Data
//-----------------------------------------------------------------------
volatile uint8_t* buffers[2];

//-----------------------------------------------------------------------
// Loop Data
//-----------------------------------------------------------------------
volatile uint8_t edma_currBuffer;
volatile uint8_t edma_nextTCD;
volatile uint8_t edma_count = 1;
volatile uint8_t edma_old_count = 0;
volatile uint8_t edma_buffer_ready = 0;

//-----------------------------------------------------------------------
// Initialize timer modules
//-----------------------------------------------------------------------
void edma_init(uint16_t transferSize)
{
    // Init eDMA modules.
    edma_state_t edmaState;
    EDMA_DRV_Init(&edmaState, &edmaUserConfig);

	// EDMA channel request.
	EDMA_DRV_RequestChannel(0, kDmaRequestMux0FTM2Channel1, &chnState);

	// Configure single end descriptor chain.
	EDMA_DRV_ConfigLoopTransfer(&chnState, TCDMem, kEDMAMemoryToPeripheral, 0x00, (uint32_t)&GPIOC_PDOR,
		1, 				//EDMA_BYTES_PER_SAMPLE
		transferSize, 	//EDMA_BYTES_PER_MINOR_LOOP
		transferSize*2,	//EDMA_TOTAL_BYTES
		2				//EDMA_TCDS_AMOUNT
	);

	// Store true address for each TCD
	edma_software_tcd_t* stcd = (edma_software_tcd_t*) STCD_ADDR(TCDMem);
	tcd0 = &(stcd[0]);
	tcd1 = &(stcd[1]);

	// Disable interrupts
	tcd0->CSR &= ~(DMA_CSR_INTHALF_MASK | DMA_CSR_INTMAJOR_MASK);
	tcd1->CSR &= ~(DMA_CSR_INTHALF_MASK | DMA_CSR_INTMAJOR_MASK);
	EDMA_HAL_SetErrorIntCmd((DMA_Type*)DMA_BASE, false, 0);
}

//-----------------------------------------------------------------------
// Sets the two buffers to loop through in a brightness pattern
//-----------------------------------------------------------------------
void edma_setBuffers(uint8_t* buffer0, uint8_t* buffer1)
{
	// Store buffers
	buffers[0] = buffer0;
	buffers[1] = buffer1;
}

//-----------------------------------------------------------------------
// Start the DMA looping transfer
//-----------------------------------------------------------------------
void edma_start()
{
	// Set the first buffers to transfer
	tcd0->SADDR = (uint32_t)(buffers[0]);
	tcd1->SADDR = (uint32_t)(buffers[0]);

	// Reset data
	edma_currBuffer = 0;
	edma_nextTCD = 0;

	// Start transfer.
	EDMA_DRV_StartChannel(&chnState);
}

//-----------------------------------------------------------------------
// Gets the current buffer being looped through in a brightness pattern
//-----------------------------------------------------------------------
uint8_t edma_getCurrentBuffer()
{
	return edma_currBuffer;
}

//-----------------------------------------------------------------------
// Gets the next buffer to be looped through in a brightness pattern
//-----------------------------------------------------------------------
uint8_t edma_getNextBuffer()
{
	if(edma_currBuffer == 0) return 1;
	return 0;
}

//-----------------------------------------------------------------------
// Update Settings
//-----------------------------------------------------------------------
void edma_updateSettings(uint8_t settings)
{
	//reset
	if((settings & 0x20) == 0x20) {
		s_mode = MODE_SPLASH;
		s_brightness = 0x03;
		splash_draw();
	}

	//standby
	else if((settings & 0x10) == 0x10) {
		s_mode = MODE_STANDBY;
		s_subMode = 0;
		spi_hold = HOLD_COUNT_1S;
		s_brightness = 0x03;
		splash_draw();
	}

	//brightness
	else {
		s_brightness = (settings & 0x03);
	}
}

//-----------------------------------------------------------------------
// Flips buffers if ready
//-----------------------------------------------------------------------
void edma_flipBuffers()
{
	if(edma_buffer_ready == 1) {
		uint8_t *tmp = dataBuffer_active;
		dataBuffer_active = dataBuffer_fill;
		dataBuffer_fill = tmp;

		edma_buffer_ready = 0;
	}
}

//-----------------------------------------------------------------------
// Fetch data from RX buffer
//-----------------------------------------------------------------------
void edma_spiFetch()
{
	while(1)
	{
		// Check RFDR flag
		if((SPI_RD_SR(spi_baseAddr) & SPI_SR_RXCTR_MASK) != 0)
		{
			// Buffer is now dirty
			edma_buffer_ready = 0;

			// Read data from POPR
			uint8_t val = DSPI_HAL_ReadData(spi_baseAddr);
			if(fillIndex < DATA_BUFFER_SIZE) {
				dataBuffer_fill[fillIndex] = val;
				fillIndex++;
			}

			// Clear the strike out counter
			spi_strike = 0;
		}
		else
		{
			// Tick the strike out counter
			spi_strike++;
			if(spi_strike > STRIKE_OUT_COUNT) {

				// Hold timer
				if(spi_hold > 0) spi_hold--;

				// Screen black out
				if(s_mode == MODE_DRAW) {
					spi_black++;
					if(spi_black > BLACK_OUT_COUNT) {
						for(uint32_t i=0; i<DATA_BUFFER_SIZE; i++) dataBuffer_active[i] = 0x00;
						spi_black = 0;

					}
				}

				// Standby Routine
				else if(s_mode == MODE_STANDBY) {
					if(s_subMode == 0) {
						if(spi_hold == 0 && spi_getClockLine() == 0) {
							spi_hold = HOLD_COUNT_2S;
							s_subMode = 1;
						}
					} else if(s_subMode == 1 && spi_hold == 0) {
						s_subMode = 2;
						s_brightness = 0x00;
						splash_draw();
					}
				}

				if(fillIndex > 0) {

					// Determine the transfer type
					if(dataBuffer_fill[fillIndex-1] != SYNC_BYTE || fillIndex != 1536+2) {

						// Incomplete transfer (only reset spi if sync byte was wrong)
						if(fillIndex == 1536+2) spi_reset();

					} else {

						// Switch to draw mode
						s_mode = MODE_DRAW;
						spi_black = 0;

						// Update metadata
						edma_updateSettings(dataBuffer_fill[fillIndex-2]);

						// Buffer transfer
						if(s_mode != MODE_SPLASH && s_mode != MODE_STANDBY) edma_buffer_ready = 1;
					}

				}

				fillIndex = 0;
				spi_strike = 0;
			}

			break;
		}
	}
}

//-----------------------------------------------------------------------
// Setup interrupt handler for the edma
//-----------------------------------------------------------------------
void DMA0_IRQHandler(void)
{
    EDMA_HAL_ClearIntStatusFlag((DMA_Type*)DMA_BASE, 0);
}

//-----------------------------------------------------------------------
// Setup interrupt handler for each time a buffer transfer triggers
//-----------------------------------------------------------------------
void FTM2_IRQHandler(void)
{
	FTM_HAL_ClearChnEventFlag(ftm2Base, 0);

	ftm_setCount(edma_old_count);
	edma_old_count = edma_count;

	ftm_disableOutput();
	ftm_enableOutput();

	edma_count++;
	if(edma_count >= 10) {
		edma_count = 0;
		edma_currBuffer ^= 0x01;
	}

	if(edma_nextTCD == 1) {
		tcd1->SADDR = (uint32_t)(&(buffers[edma_currBuffer][odr10[edma_count]]));
		edma_nextTCD = 0;
	} else {
		tcd0->SADDR = (uint32_t)(&(buffers[edma_currBuffer][odr10[edma_count]]));
		edma_nextTCD = 1;
	}
}
