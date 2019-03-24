//-----------------------------------------------------------------------
//  Includes
//-----------------------------------------------------------------------
#include "sys.h"

//-----------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------
#define TRANSFER_SIZE_12 64*4
#define TRANSFER_SIZE_24 32*4
#define ROW_BUFFER_SIZE 1536
#define DATA_BUFFER_SIZE 1536 + 4

#define DEF_BRIGHTNESS 0x03
#define DEF_COL_SIZE 0x20
#define DEF_ROW_SIZE 0x10

#define DEF_SPLASH_ANIM_SPEED 50
#define DEF_TURN180

#define MODE_DRAW 0
#define MODE_SPLASH 1
#define MODE_STANDBY 2

//-----------------------------------------------------------------------
// Flags
//-----------------------------------------------------------------------
volatile uint8_t f_lowPower = 1;
volatile uint8_t f_splashScreen = 1;

//-----------------------------------------------------------------------
// Settings
//-----------------------------------------------------------------------
volatile uint8_t s_brightness = DEF_BRIGHTNESS;
volatile uint8_t s_mode = MODE_DRAW;
volatile uint8_t s_subMode = 0;

//-----------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------
volatile uint8_t dataBuffers[2][DATA_BUFFER_SIZE];
volatile uint8_t* dataBuffer_active = dataBuffers[0];
volatile uint8_t* dataBuffer_fill = dataBuffers[1];
volatile uint32_t fillIndex;

//-----------------------------------------------------------------------
//  Additional Includes
//-----------------------------------------------------------------------
#include "const.h"
#include "gpio.h"
#include "ftm.h"
#include "spi.h"
#include "edma.h"

//-----------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------
// Row output buffers
volatile uint8_t rowBuffers[2][ROW_BUFFER_SIZE];

// Data buffer
volatile uint8_t dataLastRow;
volatile uint8_t dataCurRow;
volatile uint8_t framestep = 0;
volatile uint8_t frame = 0;

// Common
uint32_t i, j, k, l;
volatile uint32_t t0, t1;

//-----------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------
int main(void)
{
	// Initialize chip (75MHz clock)
	sys_init();

    // Setup GPIO, TIMERS, DMA, SPI
    gpio_init();
	ftm_init();
	edma_init(TRANSFER_SIZE_24);
	spi_init();

	// Get flag states from jumpers
	if(GPIO_HAL_ReadPinInput(GPIOE,30UL) > 0) f_lowPower = 0; //JP1
	if(GPIO_HAL_ReadPinInput(GPIOE,25UL) > 0) f_splashScreen = 0; //JP2

	// Blank buffer data
	if(f_splashScreen > 0) s_mode = MODE_SPLASH;
	splash_draw();
	for(i=0; i<ROW_BUFFER_SIZE; i++) {
		rowBuffers[0][i] = 0x00;
	}

    // Set buffers and start transfer
	spi_reset();
    edma_setBuffers(rowBuffers[0], rowBuffers[1]);
	ftm_start();
	edma_start();

	// Main Loop
	dataLastRow = 0;
	dataCurRow = 0;
	uint8_t nextBuffer = edma_getNextBuffer();
	for (;;)
	{
		// Determine the buffer to work with
		uint32_t* rowBuffer = rowBuffers[nextBuffer];

		// Fill rowBuffer
#ifdef DEF_TURN180
		uint32_t indexStart = (((DEF_ROW_SIZE>>1)-1)-dataCurRow)*DEF_COL_SIZE;
#else
		uint32_t indexStart = dataCurRow*DEF_COL_SIZE;
#endif
		uint32_t indexHalf = (DEF_ROW_SIZE*DEF_COL_SIZE*3) >> 1;
		for(i=0; i<DEF_COL_SIZE; i++) {

			// colors
			uint16_t index;
#ifdef DEF_TURN180
			index = (indexStart+((DEF_COL_SIZE-1)-i))*3 + indexHalf;
#else
			index = (indexStart+((DEF_COL_SIZE-1)-i))*3;
#endif
			uint16_t r1, r2, g1, g2, b1, b2;
			if(f_lowPower > 0) {
				if(s_brightness > 0x01) {
					if(s_brightness > 0x02) {
						r1 = gamma8x10_436[dataBuffer_active[index+0]];
						g1 = gamma8x10_436[dataBuffer_active[index+1]];
						b1 = gamma8x10_436[dataBuffer_active[index+2]];
					} else {
						r1 = (gamma8x10_699[dataBuffer_active[index+0]])>>1;
						g1 = (gamma8x10_699[dataBuffer_active[index+1]])>>1;
						b1 = (gamma8x10_699[dataBuffer_active[index+2]])>>1;
					}
				} else {
					if(s_brightness > 0x00) {
						r1 = gamma8x10_300[dataBuffer_active[index+0]];
						g1 = gamma8x10_300[dataBuffer_active[index+1]];
						b1 = gamma8x10_300[dataBuffer_active[index+2]];
					} else {
						r1 = (gamma8x10[dataBuffer_active[index+0]])>>2;
						g1 = (gamma8x10[dataBuffer_active[index+1]])>>2;
						b1 = (gamma8x10[dataBuffer_active[index+2]])>>2;
					}
				}
			} else {
				if(s_brightness > 0x01) {
					if(s_brightness > 0x02) {
						r1 = gamma8x10[dataBuffer_active[index+0]];
						g1 = gamma8x10[dataBuffer_active[index+1]];
						b1 = gamma8x10[dataBuffer_active[index+2]];
					} else {
						r1 = gamma8x10_699[dataBuffer_active[index+0]];
						g1 = gamma8x10_699[dataBuffer_active[index+1]];
						b1 = gamma8x10_699[dataBuffer_active[index+2]];
					}
				} else {
					if(s_brightness > 0x00) {
						r1 = gamma8x10_436[dataBuffer_active[index+0]];
						g1 = gamma8x10_436[dataBuffer_active[index+1]];
						b1 = gamma8x10_436[dataBuffer_active[index+2]];
					} else {
						r1 = (gamma8x10[dataBuffer_active[index+0]])>>2;
						g1 = (gamma8x10[dataBuffer_active[index+1]])>>2;
						b1 = (gamma8x10[dataBuffer_active[index+2]])>>2;
					}
				}
			}
#ifdef DEF_TURN180
			index -= indexHalf;
#else
			index += indexHalf;
#endif
			if(f_lowPower > 0) {
				if(s_brightness > 0x01) {
					if(s_brightness > 0x02) {
						r2 = gamma8x10_436[dataBuffer_active[index+0]];
						g2 = gamma8x10_436[dataBuffer_active[index+1]];
						b2 = gamma8x10_436[dataBuffer_active[index+2]];
					} else {
						r2 = (gamma8x10_699[dataBuffer_active[index+0]])>>1;
						g2 = (gamma8x10_699[dataBuffer_active[index+1]])>>1;
						b2 = (gamma8x10_699[dataBuffer_active[index+2]])>>1;
					}
				} else {
					if(s_brightness > 0x00) {
						r2 = gamma8x10_300[dataBuffer_active[index+0]];
						g2 = gamma8x10_300[dataBuffer_active[index+1]];
						b2 = gamma8x10_300[dataBuffer_active[index+2]];
					} else {
						r2 = (gamma8x10[dataBuffer_active[index+0]])>>2;
						g2 = (gamma8x10[dataBuffer_active[index+1]])>>2;
						b2 = (gamma8x10[dataBuffer_active[index+2]])>>2;
					}
				}
			} else {
				if(s_brightness > 0x01) {
					if(s_brightness > 0x02) {
						r2 = gamma8x10[dataBuffer_active[index+0]];
						g2 = gamma8x10[dataBuffer_active[index+1]];
						b2 = gamma8x10[dataBuffer_active[index+2]];
					} else {
						r2 = gamma8x10_699[dataBuffer_active[index+0]];
						g2 = gamma8x10_699[dataBuffer_active[index+1]];
						b2 = gamma8x10_699[dataBuffer_active[index+2]];
					}
				} else {
					if(s_brightness > 0x00) {
						r2 = gamma8x10_436[dataBuffer_active[index+0]];
						g2 = gamma8x10_436[dataBuffer_active[index+1]];
						b2 = gamma8x10_436[dataBuffer_active[index+2]];
					} else {
						r2 = (gamma8x10[dataBuffer_active[index+0]])>>2;
						g2 = (gamma8x10[dataBuffer_active[index+1]])>>2;
						b2 = (gamma8x10[dataBuffer_active[index+2]])>>2;
					}
				}
			}

			edma_spiFetch();

			//zero
			j = (r1 & 0x01) << 1;
			j |= (g1 & 0x01) << 2;
			j |= (b1 & 0x01) << 3;
			j |= (r2 & 0x01) << 4;
			j |= (g2 & 0x01) << 5;
			j |= (b2 & 0x01) << 6;
			k = j | 0x80;
			rowBuffer[i] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();

			//one
			j = (r1 & 0x02);
			j |= (g1 & 0x02) << 1;
			j |= (b1 & 0x02) << 2;
			j |= (r2 & 0x02) << 3;
			j |= (g2 & 0x02) << 4;
			j |= (b2 & 0x02) << 5;
			k = j | 0x80;
			rowBuffer[i+(TRANSFER_SIZE_24/4)] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();

			//two
			j = (r1 & 0x04) >> 1;
			j |= (g1 & 0x04);
			j |= (b1 & 0x04) << 1;
			j |= (r2 & 0x04) << 2;
			j |= (g2 & 0x04) << 3;
			j |= (b2 & 0x04) << 4;
			k = j | 0x80;
			rowBuffer[i+(TRANSFER_SIZE_24/4)*2] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();

			//three
			j = (r1 & 0x08) >> 2;
			j |= (g1 & 0x08) >> 1;
			j |= (b1 & 0x08);
			j |= (r2 & 0x08) << 1;
			j |= (g2 & 0x08) << 2;
			j |= (b2 & 0x08) << 3;
			k = j | 0x80;
			rowBuffer[i+(TRANSFER_SIZE_24/4)*3] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();

			//four
			j = (r1 & 0x10) >> 3;
			j |= (g1 & 0x10) >> 2;
			j |= (b1 & 0x10) >> 1;
			j |= (r2 & 0x10);
			j |= (g2 & 0x10) << 1;
			j |= (b2 & 0x10) << 2;
			k = j | 0x80;
			rowBuffer[i+(TRANSFER_SIZE_24/4)*4] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();

			//five
			j = (r1 & 0x20) >> 4;
			j |= (g1 & 0x20) >> 3;
			j |= (b1 & 0x20) >> 2;
			j |= (r2 & 0x20) >> 1;
			j |= (g2 & 0x20);
			j |= (b2 & 0x20) << 1;
			k = j | 0x80;
			rowBuffer[i+(TRANSFER_SIZE_24/4)*5] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();

			//six
			j = (r1 & 0x40) >> 5;
			j |= (g1 & 0x40) >> 4;
			j |= (b1 & 0x40) >> 3;
			j |= (r2 & 0x40) >> 2;
			j |= (g2 & 0x40) >> 1;
			j |= (b2 & 0x40);
			k = j | 0x80;
			rowBuffer[i+(TRANSFER_SIZE_24/4)*6] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();

			//seven
			j = (r1 & 0x80) >> 6;
			j |= (g1 & 0x80) >> 5;
			j |= (b1 & 0x80) >> 4;
			j |= (r2 & 0x80) >> 3;
			j |= (g2 & 0x80) >> 2;
			j |= (b2 & 0x80) >> 1;
			k = j | 0x80;
			rowBuffer[i+(TRANSFER_SIZE_24/4)*7] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();

			//eight
			j = (r1 & 0x100) >> 7;
			j |= (g1 & 0x100) >> 6;
			j |= (b1 & 0x100) >> 5;
			j |= (r2 & 0x100) >> 4;
			j |= (g2 & 0x100) >> 3;
			j |= (b2 & 0x100) >> 2;
			k = j | 0x80;
			rowBuffer[i+(TRANSFER_SIZE_24/4)*8] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();

			//nine
			j = (r1 & 0x200) >> 8;
			j |= (g1 & 0x200) >> 7;
			j |= (b1 & 0x200) >> 6;
			j |= (r2 & 0x200) >> 5;
			j |= (g2 & 0x200) >> 4;
			j |= (b2 & 0x200) >> 3;
			k = j | 0x80;
			rowBuffer[i+(TRANSFER_SIZE_24/4)*9] = (j<<24) + (j<<16) + (k<<8) + k;

			edma_spiFetch();
		}

		// Update Splash animation
		if((s_mode == MODE_SPLASH || (s_mode == MODE_STANDBY && s_subMode < 2)) && fillIndex == 0) {
			framestep++;
			if(framestep > DEF_SPLASH_ANIM_SPEED) {
				framestep = 0;
				frame++; if(frame == prog_frames) frame = 0;

				uint32_t offset = (27*3);
				uint32_t next = (5*3);
				i=(27*3); l=i+next; j=0;
				while(j<prog_size) {
					uint8_t mixed = prog[frame][j]; j++;
					for(k=0; k<8; k+=2) {
						const uint8_t* color = color_palette[(mixed & (0xC0 >> k)) >> (6-k)];
						dataBuffer_active[i] = color[0]; i++; if(i==l){ i+=offset; l=i+next; }
						dataBuffer_active[i] = color[1]; i++; if(i==l){ i+=offset; l=i+next; }
						dataBuffer_active[i] = color[2]; i++; if(i==l){ i+=offset; l=i+next; }
					}
				}
			}
		}

		// Wait until the buffer flips
		while(ftm_getCountLeft() > 400) edma_spiFetch();
		nextBuffer = edma_getNextBuffer();
		ftm_disableOutput();

		// Move to next row
		dataLastRow = dataCurRow;
		dataCurRow++;
		if(dataCurRow >= (DEF_ROW_SIZE>>1)) {
			dataCurRow = 0;
			edma_flipBuffers();
		}
		GPIO_WR_PDOR(GPIOD, (dataLastRow << 4U));
	}
}
