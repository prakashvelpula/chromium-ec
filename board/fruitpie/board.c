/* Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/* Fruitpie board configuration */

#include "adc.h"
#include "adc_chip.h"
#include "common.h"
#include "console.h"
#include "gpio.h"
#include "hooks.h"
#include "i2c.h"
#include "registers.h"
#include "task.h"
#include "util.h"

void rohm_event(enum gpio_signal signal)
{
	ccprintf("ROHM!\n");
}

void vbus_event(enum gpio_signal signal)
{
	ccprintf("VBUS!\n");
}

void tsu_event(enum gpio_signal signal)
{
	ccprintf("TSU!\n");
}

#include "gpio_list.h"

/* Initialize board. */
static void board_init(void)
{
}
DECLARE_HOOK(HOOK_INIT, board_init, HOOK_PRIO_DEFAULT);

/* Pins with alternate functions */
const struct gpio_alt_func gpio_alt_funcs[] = {
	{GPIO_B, 0x2000, 0, MODULE_USB_PD},/* SPI2: SCK(PB13) */
	{GPIO_B, 0x0200, 2, MODULE_USB_PD},/* TIM17_CH1: PB9) */
	{GPIO_A, 0xC000, 1, MODULE_UART},  /* USART2: PA14/PA15 */
	{GPIO_B, 0x0cc0, 1, MODULE_I2C},   /* I2C SLAVE:PB6/7 MASTER:PB10/11 */
};
const int gpio_alt_funcs_count = ARRAY_SIZE(gpio_alt_funcs);

/* ADC channels */
const struct adc_t adc_channels[] = {
	/* USB PD CC lines sensing. Converted to mV (3300mV/4096). */
	[ADC_CH_CC1_PD] = {"CC1_PD", 3300, 4096, 0, STM32_AIN(0)},
	[ADC_CH_CC2_PD] = {"CC2_PD", 3300, 4096, 0, STM32_AIN(4)},
};
BUILD_ASSERT(ARRAY_SIZE(adc_channels) == ADC_CH_COUNT);

/* I2C ports */
const struct i2c_port_t i2c_ports[] = {
	{"master", I2C_PORT_MASTER, 100,
		GPIO_MASTER_I2C_SCL, GPIO_MASTER_I2C_SDA},
};
const unsigned int i2c_ports_used = ARRAY_SIZE(i2c_ports);

void board_set_usb_mux(enum typec_mux mux)
{
	/* reset everything */
	gpio_set_level(GPIO_SS1_EN_L, 1);
	gpio_set_level(GPIO_SS2_EN_L, 1);
	gpio_set_level(GPIO_DP_MODE, 0);
	gpio_set_level(GPIO_SS1_USB_MODE_L, 1);
	gpio_set_level(GPIO_SS2_USB_MODE_L, 1);
	switch (mux) {
	case TYPEC_MUX_NONE:
		/* everything is already disabled, we can return */
		return;
	case TYPEC_MUX_USB1:
		gpio_set_level(GPIO_SS1_USB_MODE_L, 0);
		break;
	case TYPEC_MUX_USB2:
		gpio_set_level(GPIO_SS2_USB_MODE_L, 0);
		break;
	case TYPEC_MUX_DP1:
		gpio_set_level(GPIO_DP_POLARITY_L, 1);
		gpio_set_level(GPIO_DP_MODE, 1);
		break;
	case TYPEC_MUX_DP2:
		gpio_set_level(GPIO_DP_POLARITY_L, 0);
		gpio_set_level(GPIO_DP_MODE, 1);
		break;
	}
	gpio_set_level(GPIO_SS1_EN_L, 0);
	gpio_set_level(GPIO_SS2_EN_L, 0);
}

static int command_typec(int argc, char **argv)
{
	const char * const mux_name[] = {"none", "usb1", "usb2", "dp1", "dp2"};

	if (argc < 2) {
		/* dump current state */
		ccprintf("CC1 %d mV  CC2 %d mV\n",
			adc_read_channel(ADC_CH_CC1_PD),
			adc_read_channel(ADC_CH_CC2_PD));
		ccprintf("DP %d Polarity %d\n", gpio_get_level(GPIO_DP_MODE),
			!!gpio_get_level(GPIO_DP_POLARITY_L) + 1);
		ccprintf("Superspeed %s\n",
			gpio_get_level(GPIO_SS1_EN_L) ? "None" :
			(gpio_get_level(GPIO_DP_MODE) ? "DP" :
			(!gpio_get_level(GPIO_SS1_USB_MODE_L) ? "USB1" : "USB2")
			));
		return EC_SUCCESS;
	}

	if (!strcasecmp(argv[1], "mux")) {
		enum typec_mux mux = TYPEC_MUX_NONE;
		int i;

		if (argc < 3)
			return EC_ERROR_PARAM2;

		for (i = 0; i < ARRAY_SIZE(mux_name); i++)
			if (!strcasecmp(argv[2], mux_name[i]))
				mux = i;
		board_set_usb_mux(mux);
		return EC_SUCCESS;
	} else {
		return EC_ERROR_PARAM1;
	}

	return EC_ERROR_UNKNOWN;
}
DECLARE_CONSOLE_COMMAND(typec, command_typec,
			"[mux none|usb1|usb2|dp1|d2]",
			"Control type-C connector",
			NULL);
