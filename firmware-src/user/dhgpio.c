/*
 * dhgpio.c
 *
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: GPIO module
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include "user_config.h"
#include "dhdebug.h"

#include "dhgpio.h"

int ICACHE_FLASH_ATTR dhgpio_write(unsigned int set_mask, unsigned int unset_mask) {
	const uint32 pins = set_mask | unset_mask;
	dhdebug("GPIO set high at 0x%X, set low at 0x%X", set_mask, unset_mask);
	if((pins | DHGPIO_SUITABLE_PINS) != DHGPIO_SUITABLE_PINS)
		return 0;
	if(pins & PIN_GPIOO)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	if(pins & PIN_GPIO1)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
	if(pins & PIN_GPIO2)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	if(pins & PIN_GPIO3)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
	if(pins & PIN_GPIO4)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	if(pins & PIN_GPIO5)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	if(pins & PIN_GPIO12)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	if(pins & PIN_GPIO13)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	if(pins & PIN_GPIO14)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	if(pins & PIN_GPIO15)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	gpio_output_set(set_mask, unset_mask, pins, 0);
	return 1;
}

int ICACHE_FLASH_ATTR dhgpio_init(unsigned int init_mask, unsigned int pollup_mask, unsigned int nopoll_mask) {
	if(pollup_mask & nopoll_mask)
		return 0;
	if( (init_mask | pollup_mask | nopoll_mask | DHGPIO_SUITABLE_PINS) != DHGPIO_SUITABLE_PINS)
		return 0;
	// init
	gpio_output_set(0, 0, 0, init_mask | pollup_mask | nopoll_mask);
	// pullup
	if(pollup_mask & PIN_GPIOO)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
	if(pollup_mask & PIN_GPIO1)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_U0TXD_U);
	if(pollup_mask & PIN_GPIO2)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
	if(pollup_mask & PIN_GPIO3)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_U0RXD_U);
	if(pollup_mask & PIN_GPIO4)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);
	if(pollup_mask & PIN_GPIO5)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
	if(pollup_mask & PIN_GPIO12)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_MTDI_U);
	if(pollup_mask & PIN_GPIO13)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_MTCK_U);
	if(pollup_mask & PIN_GPIO14)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_MTMS_U);
	if(pollup_mask & PIN_GPIO15)
		PIN_PULLUP_EN(PERIPHS_IO_MUX_MTDO_U);

	if(nopoll_mask & PIN_GPIOO)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO0_U);
	if(nopoll_mask & PIN_GPIO1)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
	if(nopoll_mask & PIN_GPIO2)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO2_U);
	if(nopoll_mask & PIN_GPIO3)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0RXD_U);
	if(nopoll_mask & PIN_GPIO4)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO4_U);
	if(nopoll_mask & PIN_GPIO5)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO5_U);
	if(nopoll_mask & PIN_GPIO12)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDI_U);
	if(nopoll_mask & PIN_GPIO13)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTCK_U);
	if(nopoll_mask & PIN_GPIO14)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTMS_U);
	if(nopoll_mask & PIN_GPIO15)
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDO_U);
	return 1;
}

unsigned int ICACHE_FLASH_ATTR dhgpio_read() {
	return gpio_input_get() & DHGPIO_SUITABLE_PINS;
}

int ICACHE_FLASH_ATTR dhgpio_int(unsigned int disable_mask, unsigned int rising_mask, unsigned int falling_mask, unsigned int both_mask, unsigned int low_mask, unsigned int high_mask) {
	unsigned int tmp = disable_mask;
	if(tmp & rising_mask)
		return 0;
	tmp |= rising_mask;
	if(tmp & falling_mask)
		return 0;
	tmp |= falling_mask;
	if(tmp & both_mask)
		return 0;
	tmp |= both_mask;
	if(tmp & low_mask)
		return 0;
	tmp |= low_mask;
	if(tmp & high_mask)
		return 0;
	if( (disable_mask | rising_mask | falling_mask | both_mask | low_mask | high_mask | DHGPIO_SUITABLE_PINS) != DHGPIO_SUITABLE_PINS)
		return 0;
	int i;
	for(i = 0; i < 32; i++) {
		const unsigned int pin = 1 << i;
		if(pin & DHGPIO_SUITABLE_PINS == 0)
			continue;
		else if(pin & disable_mask)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_DISABLE);
		else if(pin & falling_mask)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_POSEDGE);
		else if(pin & rising_mask)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_NEGEDGE);
		else if(pin & both_mask)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_ANYEDGE);
		else if(pin & low_mask)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_LOLEVEL);
		else if(pin & high_mask)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_HILEVEL);
	}
	return 1;
}

int ICACHE_FLASH_ATTR dhgpio_read_to_json(char *out, unsigned int value) {
	int len = os_sprintf(out, "{");
	int i;
	for(i = 0; i < 32; i++) {
		const unsigned int pin = 1 << i;
		const pinvalue = (value & pin) ? 1 : 0;
		if(DHGPIO_SUITABLE_PINS & pin) {
			len += os_sprintf(&out[len], (i == 0) ? "\"%d\":\"%d\"" : ", \"%d\":\"%d\"", i, pinvalue);
		}
	}
	return len + os_sprintf(&out[len], "}");
}
