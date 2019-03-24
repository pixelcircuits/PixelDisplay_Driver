//-----------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------
#define DSPI_SLAVE_INSTANCE (0)
#define STRIKE_OUT_COUNT (500)
#define HOLD_COUNT_1S (75)
#define HOLD_COUNT_2S (800)
#define BLACK_OUT_COUNT (1000)
#define SYNC_BYTE 0x01

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const dspi_data_format_config_t dataConfig = {
		.bitsPerFrame = 8,
		.clkPhase     = kDspiClockPhase_FirstEdge,
		.clkPolarity  = kDspiClockPolarity_ActiveHigh
};

//-----------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------
extern SPI_Type* const g_dspiBase[SPI_INSTANCE_COUNT];
volatile SPI_Type* spi_baseAddr;
volatile uint32_t spi_strike = 0;
volatile uint32_t spi_black = 0;
volatile uint32_t spi_hold = 0;

//-----------------------------------------------------------------------
// Initialize SPI
//-----------------------------------------------------------------------
void spi_init()
{
	// Get SPI base
    spi_baseAddr = g_dspiBase[DSPI_SLAVE_INSTANCE];

    // Set SPI pins
    PORT_HAL_SetMuxMode(PORTE,16UL,kPortMuxAlt2); //cs
    PORT_HAL_SetMuxMode(PORTE,17UL,kPortMuxAlt2); //sck
    PORT_HAL_SetMuxMode(PORTE,18UL,kPortMuxAlt2); //sout
    PORT_HAL_SetMuxMode(PORTE,19UL,kPortMuxAlt2); //sin

    // Enable clock for DSPI
    CLOCK_SYS_EnableSpiClock(DSPI_SLAVE_INSTANCE);

    // Reset the DSPI module, which also disables the DSPI module
    DSPI_HAL_Init(spi_baseAddr);

    // Set to slave mode.
    DSPI_HAL_SetMasterSlaveMode(spi_baseAddr, kDspiSlave);

    // Set data format
    DSPI_HAL_SetDataFormat(spi_baseAddr, kDspiCtar0, &dataConfig);

    // DSPI system enable
    DSPI_HAL_Enable(spi_baseAddr);

    // Enable FIFO operation.
    DSPI_HAL_SetFifoCmd(spi_baseAddr, 0, 1);

	// Start the transfer process
	DSPI_HAL_StartTransfer(spi_baseAddr);
}

//-----------------------------------------------------------------------
// Reset SPI
//-----------------------------------------------------------------------
void spi_reset()
{
	// Flush remaining bits from SPI
	GPIO_DRV_OutputPinInit(&SPI_CLK_OUT);
	while((SPI_RD_SR(spi_baseAddr) & SPI_SR_RXCTR_MASK) == 0) {
		GPIO_WR_PDOR(GPIOB, 0x01);
		GPIO_WR_PDOR(GPIOB, 0x00);
	}
	GPIO_DRV_InputPinInit(&SPI_CLK_IN);
	DSPI_HAL_SetFlushFifoCmd(spi_baseAddr, 1, 1);
}

//-----------------------------------------------------------------------
// Gets state of the SPI Clock line
//-----------------------------------------------------------------------
uint8_t spi_getClockLine()
{
	return GPIO_HAL_ReadPinInput(GPIOB,0UL);
}

//-----------------------------------------------------------------------
// Gets number of bytes in RX buffer
//-----------------------------------------------------------------------
uint8_t spi_getRXByteCount()
{
	return (uint8_t)((SPI_RD_SR(spi_baseAddr) & SPI_SR_RXCTR_MASK) >> SPI_SR_RXCTR_SHIFT);
}
