//-----------------------------------------------------------------------
//  Includes
//-----------------------------------------------------------------------
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_clock_manager.h"
#include "fsl_interrupt_manager.h"
#include "fsl_power_manager.h"
#include "fsl_adc16_driver.h"
#include "fsl_cmp_driver.h"
#include "fsl_crc_driver.h"
#include "fsl_dac_driver.h"
#include "fsl_dspi_edma_master_driver.h"
#include "fsl_dspi_edma_slave_driver.h"
#include "fsl_dspi_master_driver.h"
#include "fsl_dspi_slave_driver.h"
#include "fsl_edma_driver.h"
#include "fsl_ewm_driver.h"
#include "fsl_ftm_driver.h"
#include "fsl_gpio_driver.h"
#include "fsl_i2c_master_driver.h"
#include "fsl_i2c_slave_driver.h"
#include "fsl_lptmr_driver.h"
#include "fsl_pdb_driver.h"
#include "fsl_uart_driver.h"
#include "fsl_uart_edma_driver.h"
#include "fsl_wdog_driver.h"

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
/* Core clock = 75MHz. */
const clock_manager_user_config_t g_defaultClockConfigRun =
{
    .mcgConfig =
    {
        .mcg_mode           = kMcgModeFEE,	// Work in FEE mode.
        .irclkEnable        = true,  		// MCGIRCLK enable.
        .irclkEnableInStop  = false, 		// MCGIRCLK disable in STOP mode.
        .ircs               = kMcgIrcSlow,	// Select IRC32k.
        .fcrdiv             = 0U,    		// FCRDIV is 0.

        .frdiv   = 3U,
        .drs     = kMcgDcoRangeSelMidHigh,  // Mid high frequency range
        .dmx32   = kMcgDmx32Default,        // DCO has a default range of 25%
    },
    .simConfig =
    {
        .er32kSrc  = kClockEr32kSrcLpo,     // ERCLK32K selection, use LPO.
        .outdiv1   = 0U,
        .outdiv4   = 2U,
        .outdiv5   = 0U,
        .outdiv5Enable = false,
    },
    .oscerConfig =
    {
        .enable       = true,  // OSCERCLK enable.
        .enableInStop = false, // OSCERCLK disable in STOP mode.
    }
};
/* Core clock = 25MHz. */
const clock_manager_user_config_t g_defaultClockConfigSlow =
{
    .mcgConfig =
    {
        .mcg_mode           = kMcgModeFEE,	// Work in FEE mode.
        .irclkEnable        = true,  		// MCGIRCLK enable.
        .irclkEnableInStop  = false, 		// MCGIRCLK disable in STOP mode.
        .ircs               = kMcgIrcSlow,	// Select IRC32k.
        .fcrdiv             = 0U,    		// FCRDIV is 0.

        .frdiv   = 3U,
        .drs     = kMcgDcoRangeSelMidHigh,  // Mid high frequency range
        .dmx32   = kMcgDmx32Default,        // DCO has a default range of 25%
    },
    .simConfig =
    {
        .er32kSrc  = kClockEr32kSrcLpo,     // ERCLK32K selection, use LPO.
        .outdiv1   = 2U,
        .outdiv4   = 2U,
        .outdiv5   = 0U,
        .outdiv5Enable = false,
    },
    .oscerConfig =
    {
        .enable       = true,  // OSCERCLK enable.
        .enableInStop = false, // OSCERCLK disable in STOP mode.
    }
};
/* Core clock = 4MHz. */
const clock_manager_user_config_t g_defaultClockConfigVlpr =
{
    .mcgConfig =
    {
        .mcg_mode           = kMcgModeBLPI,   // Work in BLPI mode.
        .irclkEnable        = true,  // MCGIRCLK enable.
        .irclkEnableInStop  = false, // MCGIRCLK disable in STOP mode.
        .ircs               = kMcgIrcFast, // Select IRC4M.
        .fcrdiv             = 0U,    // FCRDIV is 0.

        .frdiv   = 0U,
        .drs     = kMcgDcoRangeSelLow,  // Low frequency range
        .dmx32   = kMcgDmx32Default,    // DCO has a default range of 25%
    },
    .simConfig =
    {
        .er32kSrc  = kClockEr32kSrcLpo,     // ERCLK32K selection, use LPO.
        .outdiv1   = 1U, //0
        .outdiv4   = 2U, //4
        .outdiv5   = 0U, //0
        .outdiv5Enable = false,
    },
    .oscerConfig =
    {
        .enable       = true,  // OSCERCLK enable.
        .enableInStop = false, // OSCERCLK disable in STOP mode.
    }
};

//-----------------------------------------------------------------------
// Initialization Function
//-----------------------------------------------------------------------
void sys_init()
{
    /* Enable clocks on all ports. */
    CLOCK_SYS_EnablePortClock(PORTA_IDX);
    CLOCK_SYS_EnablePortClock(PORTB_IDX);
    CLOCK_SYS_EnablePortClock(PORTC_IDX);
    CLOCK_SYS_EnablePortClock(PORTD_IDX);
    CLOCK_SYS_EnablePortClock(PORTE_IDX);

    /* Set allowed power mode, allow all. */
    SMC_HAL_SetProtection(SMC, kAllowPowerModeAll);

    /* Setup board clock source. */
    PORT_HAL_SetMuxMode(PORTA, 18, kPortPinDisabled);
    PORT_HAL_SetMuxMode(PORTA, 19, kPortPinDisabled);

    /* OSC0 configuration. */
    osc_user_config_t osc0Config =
    {
        .freq                = 10000000U,
        .hgo                 = kOscGainLow,
        .range               = kOscRangeVeryHigh,
        .erefs               = kOscSrcOsc,
        .enableCapacitor2p   = false,
        .enableCapacitor4p   = false,
        .enableCapacitor8p   = false,
        .enableCapacitor16p  = true,
    };
    CLOCK_SYS_OscInit(0U, &osc0Config);

    /* Set system clock configuration. */
    CLOCK_SYS_SetConfiguration(&g_defaultClockConfigRun);
    //CLOCK_SYS_SetConfiguration(&g_defaultClockConfigSlow);

    /* Update core clock reference. */
    SystemCoreClockUpdate();
}
