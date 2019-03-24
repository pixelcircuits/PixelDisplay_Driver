//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const gpio_output_pin_user_config_t R1 = {
  .pinName = GPIO_MAKE_PIN(GPIOC_IDX, 1U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t G1 = {
  .pinName = GPIO_MAKE_PIN(GPIOC_IDX, 2U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t B1 = {
  .pinName = GPIO_MAKE_PIN(GPIOC_IDX, 3U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t R2 = {
  .pinName = GPIO_MAKE_PIN(GPIOC_IDX, 4U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t G2 = {
  .pinName = GPIO_MAKE_PIN(GPIOC_IDX, 5U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t B2 = {
  .pinName = GPIO_MAKE_PIN(GPIOC_IDX, 6U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t CLK = {
  .pinName = GPIO_MAKE_PIN(GPIOC_IDX, 7U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t LAT = {
  .pinName = GPIO_MAKE_PIN(GPIOA_IDX, 1U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t OE = {
  .pinName = GPIO_MAKE_PIN(GPIOA_IDX, 2U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t ADR0 = {
  .pinName = GPIO_MAKE_PIN(GPIOD_IDX, 4U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t ADR1 = {
  .pinName = GPIO_MAKE_PIN(GPIOD_IDX, 5U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t ADR2 = {
  .pinName = GPIO_MAKE_PIN(GPIOD_IDX, 6U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t ADR3 = {
  .pinName = GPIO_MAKE_PIN(GPIOD_IDX, 7U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};

const gpio_input_pin_user_config_t SPI_CLK_IN = {
  .pinName = GPIO_MAKE_PIN(GPIOB_IDX, 0U),
  .config.isPullEnable = 0,
};
const gpio_output_pin_user_config_t SPI_CLK_OUT = {
  .pinName = GPIO_MAKE_PIN(GPIOB_IDX, 0U),
  .config.outputLogic = 0,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};
const gpio_output_pin_user_config_t SPI_CS = {
  .pinName = GPIO_MAKE_PIN(GPIOB_IDX, 1U),
  .config.outputLogic = 1,
  .config.slewRate = kPortSlowSlewRate,
  .config.driveStrength = kPortLowDriveStrength,
};

const gpio_input_pin_user_config_t JUMPER_POWER = {
  .pinName = GPIO_MAKE_PIN(GPIOE_IDX, 30U),
  .config.isPullEnable = 1,
  .config.pullSelect = kPortPullDown,
};
const gpio_input_pin_user_config_t JUMPER_SPLASH = {
  .pinName = GPIO_MAKE_PIN(GPIOE_IDX, 25U),
  .config.isPullEnable = 1,
  .config.pullSelect = kPortPullDown,
};

//-----------------------------------------------------------------------
// Initialize gpio modules
//-----------------------------------------------------------------------
void gpio_init()
{
	GPIO_DRV_OutputPinInit(&R1);
	GPIO_DRV_OutputPinInit(&G1);
	GPIO_DRV_OutputPinInit(&B1);
	GPIO_DRV_OutputPinInit(&R2);
	GPIO_DRV_OutputPinInit(&G2);
	GPIO_DRV_OutputPinInit(&B2);
	GPIO_DRV_OutputPinInit(&CLK);
	GPIO_DRV_OutputPinInit(&LAT);
	GPIO_DRV_OutputPinInit(&OE);

	GPIO_DRV_OutputPinInit(&ADR0);
	GPIO_DRV_OutputPinInit(&ADR1);
	GPIO_DRV_OutputPinInit(&ADR2);
	GPIO_DRV_OutputPinInit(&ADR3);

	GPIO_WR_PDOR(GPIOC, 0x00);
	GPIO_WR_PDOR(GPIOD, 0x00);
	GPIO_WR_PDOR(GPIOA, 0x00);
	GPIO_WR_PSOR(GPIOA, 1U << 2U);

	// Flags
	GPIO_DRV_InputPinInit(&JUMPER_POWER);
	GPIO_DRV_InputPinInit(&JUMPER_SPLASH);

	// SPI
	GPIO_DRV_OutputPinInit(&SPI_CS);
	GPIO_DRV_OutputPinInit(&SPI_CLK_OUT);
	GPIO_DRV_InputPinInit(&SPI_CLK_IN);
	GPIO_WR_PDOR(GPIOB, 0x00);
}
