//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const ftm_pwm_param_t flexTimer0_ChnConfig0 = {
	.mode = kFtmEdgeAlignedPWM,
	.edgeMode = kFtmHighTrue,
	.uFrequencyHZ = 1000U, 		/* dummy value, will change it later on */
	.uDutyCyclePercent = 10U, 	/* dummy value, will change it later on */
	.uFirstEdgeDelayPercent = 0U,
};
const ftm_user_config_t flexTimer0_InitConfig0 = {
	.tofFrequency      = 0U,
	.isWriteProtection = false,
	.BDMMode           = kFtmBdmMode_00,
	.syncMethod        = (uint32_t)(kFtmUseSoftwareTrig)
};

//-----------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------
#define FTM_DATA_TRANSFER 980
#define FTM_DATA_TRANSFER_OFFSET 600
#define FTM_DATA_LATCH 8
#define FTM_SMALLEST_COUNT_24 80
#define FTM_SMALLEST_COUNT_12 800

//-----------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------
volatile FTM_Type* ftm2Base;
volatile FTM_Type* ftm1Base;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const uint16_t ftm_bright_remainder_24[10] = { (0x0F-(((FTM_SMALLEST_COUNT_24<<0)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_24<<1)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_24<<2)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_24<<3)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_24<<4)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_24<<5)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_24<<6)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_24<<7)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_24<<8)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_24<<9)*0x0F)/FTM_DATA_TRANSFER)) };
const uint16_t ftm_bright_remainder_12[6] = { (0x0F-(((FTM_SMALLEST_COUNT_12<<0)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_12<<1)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_12<<2)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_12<<3)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_12<<4)*0x0F)/FTM_DATA_TRANSFER)), (0x0F-(((FTM_SMALLEST_COUNT_12<<5)*0x0F)/FTM_DATA_TRANSFER)) };

//-----------------------------------------------------------------------
// Initialize timer modules
//-----------------------------------------------------------------------
void ftm_init()
{
	ftm1Base = g_ftmBase[FTM1_IDX];
	ftm2Base = g_ftmBase[FTM2_IDX];

	/***************************
	 * Latch and Trigger Timer *
	 ***************************/

	/* Initialize the driver. */
	FTM_DRV_Init(FTM2_IDX, &flexTimer0_InitConfig0);
	FTM_DRV_SetTimeOverflowIntCmd(FTM2_IDX, false); /* disable interrupt */
	FTM_DRV_SetFaultIntCmd(FTM2_IDX, false); /* disable interrupt */
	FTM_DRV_SetClock(FTM2_IDX, kClock_source_FTM_SystemClk, kFtmDividedBy1);

	/* Configure timer. */
	FTM_HAL_ClearTimerOverflow(ftm2Base);

	/* Enable PWM mode for the channels. */
	FTM_HAL_EnablePwmMode(ftm2Base, (ftm_pwm_param_t*)&flexTimer0_ChnConfig0, 0);
	FTM_HAL_EnablePwmMode(ftm2Base, (ftm_pwm_param_t*)&flexTimer0_ChnConfig0, 1);

	/* Based on Ref manual, in PWM mode CNTIN is to be set 0. */
	FTM_HAL_SetCounterInitVal(ftm2Base, 0);

	/* Set the module counters. */
	FTM_HAL_SetMod(ftm2Base, FTM_DATA_TRANSFER);
	FTM_HAL_SetChnCountVal(ftm2Base, 0, FTM_DATA_LATCH); //lat
	FTM_HAL_SetChnCountVal(ftm2Base, 1, FTM_DATA_TRANSFER-FTM_DATA_TRANSFER_OFFSET); //trig

	/* Use PTA1 for channel 1 of FTM2. */
	PORT_HAL_SetMuxMode(PORTA, 1UL, kPortMuxAlt3);

	/* Reset routine. */
	FTM_HAL_SetCounter(ftm2Base, 0);
	FTM_HAL_ClearTimerOverflow(ftm2Base);

	/* Setup edma trigger. */
	FTM_HAL_ClearChnEventFlag(ftm2Base, 1);
	FTM_HAL_SetChnDmaCmd(ftm2Base, 1, true);
	FTM_HAL_EnableChnInt(ftm2Base, 1);

	/* Enable interrupt. */
	FTM_HAL_ClearChnEventFlag(ftm2Base, 0);
	FTM_HAL_EnableChnInt(ftm2Base, 0);

	/********************************
	 * Lower Value Brightness Timer *
	 ********************************/

	/* Initialize the driver. */
	FTM_DRV_Init(FTM1_IDX, &flexTimer0_InitConfig0);
	FTM_DRV_SetTimeOverflowIntCmd(FTM1_IDX, false); /* disable interrupt */
	FTM_DRV_SetFaultIntCmd(FTM1_IDX, false); /* disable interrupt */
	FTM_DRV_SetClock(FTM1_IDX, kClock_source_FTM_SystemClk, kFtmDividedBy1);

	/* Configure timer. */
	FTM_HAL_ClearTimerOverflow(ftm1Base);

	/* Enable PWM mode for the channels. */
	FTM_HAL_EnablePwmMode(ftm1Base, (ftm_pwm_param_t*)&flexTimer0_ChnConfig0, 0);

	/* Based on Ref manual, in PWM mode CNTIN is to be set 0. */
	FTM_HAL_SetCounterInitVal(ftm1Base, 0);

	/* Set the module counters. */
	FTM_HAL_SetMod(ftm1Base, 0x0F);
	FTM_HAL_SetChnCountVal(ftm1Base, 0, 0x00);

	/* Use PTA2 for channel 0 of FTM1. */
	PORT_HAL_SetMuxMode(PORTA, 2UL, kPortMuxAlt6);

	/* Reset routine and start. */
	FTM_HAL_SetCounter(ftm1Base, 0);
	FTM_HAL_ClearTimerOverflow(ftm1Base);
	FTM_HAL_SetClockSource(ftm1Base, kClock_source_FTM_SystemClk);
}

//-----------------------------------------------------------------------
// Sets the count of the timer
//-----------------------------------------------------------------------
void ftm_setCount(uint8_t count)
{
	uint16_t size = FTM_DATA_TRANSFER;
	size = (FTM_SMALLEST_COUNT_24 << count);
	if(size < FTM_DATA_TRANSFER)
	{
		FTM_HAL_SetClockSource(ftm1Base, kClock_source_FTM_None);
		FTM_HAL_SetChnCountVal(ftm1Base, 0, ftm_bright_remainder_24[count]);
		FTM_HAL_SetClockSource(ftm1Base, kClock_source_FTM_SystemClk);
		size = FTM_DATA_TRANSFER;
	}
	else
	{
		FTM_HAL_SetClockSource(ftm1Base, kClock_source_FTM_None);
		FTM_HAL_SetChnCountVal(ftm1Base, 0, 0x00);
		FTM_HAL_SetClockSource(ftm1Base, kClock_source_FTM_SystemClk);
	}

    FTM_HAL_SetClockSource(ftm2Base, kClock_source_FTM_None);
	FTM_HAL_SetMod(ftm2Base, size);
	FTM_HAL_SetChnCountVal(ftm2Base, 1, size-FTM_DATA_TRANSFER_OFFSET); //trig
    FTM_HAL_SetClockSource(ftm2Base, kClock_source_FTM_SystemClk);
}

//-----------------------------------------------------------------------
// Gets if the timer is just about at the end of it's bit routine
//-----------------------------------------------------------------------
uint16_t ftm_getCountLeft()
{
    return (FTM_SMALLEST_COUNT_24 << 0x09) - FTM_HAL_GetCounter(ftm2Base);
}

//-----------------------------------------------------------------------
// Disables output for the display
//-----------------------------------------------------------------------
void ftm_disableOutput()
{
	PORT_HAL_SetMuxMode(PORTA, 2UL, kPortMuxAsGpio);
}

//-----------------------------------------------------------------------
// Enables output for the display
//-----------------------------------------------------------------------
void ftm_enableOutput()
{
	PORT_HAL_SetMuxMode(PORTA, 2UL, kPortMuxAlt6);
}

//-----------------------------------------------------------------------
// Start the timer modules
//-----------------------------------------------------------------------
void ftm_start()
{
    FTM_HAL_SetClockSource(ftm2Base, kClock_source_FTM_SystemClk);
}

//-----------------------------------------------------------------------
// Stop the timer modules
//-----------------------------------------------------------------------
void ftm_stop()
{
    FTM_HAL_SetClockSource(ftm2Base, kClock_source_FTM_None);
}
