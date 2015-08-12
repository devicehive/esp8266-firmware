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
#include "dhnotification.h"
#include "dhgpio.h"
#include "dhpwm.h"
#include "dhmem.h"

#define PIN_GPIOO (1 << 0)
#define PIN_GPIO1 (1 << 1)
#define PIN_GPIO2 (1 << 2)
#define PIN_GPIO3 (1 << 3)
#define PIN_GPIO4 (1 << 4)
#define PIN_GPIO5 (1 << 5)
#define PIN_GPIO6 (1 << 6)
#define PIN_GPIO7 (1 << 7)
#define PIN_GPIO8 (1 << 8)
#define PIN_GPIO9 (1 << 9)
#define PIN_GPIO10 (1 << 10)
#define PIN_GPIO11 (1 << 11)
#define PIN_GPIO12 (1 << 12)
#define PIN_GPIO13 (1 << 13)
#define PIN_GPIO14 (1 << 14)
#define PIN_GPIO15 (1 << 15)

os_timer_t mGPIOTimer;
unsigned int mGPIOTimerTimeout = 250;
unsigned int mTriggeredIntrPins = 0;
unsigned char mGPIOTimerArmed = 0;

void ICACHE_FLASH_ATTR dhgpio_prepare_pins(unsigned int pin_mask, int disable_pwm) {
	if(disable_pwm)
		dhpwm_disable_pins(pin_mask);
	if(pin_mask & PIN_GPIOO)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	if(pin_mask & PIN_GPIO1)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
	if(pin_mask & PIN_GPIO2)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	if(pin_mask & PIN_GPIO3)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
	if(pin_mask & PIN_GPIO4)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	if(pin_mask & PIN_GPIO5)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	if(pin_mask & PIN_GPIO12)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	if(pin_mask & PIN_GPIO13)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	if(pin_mask & PIN_GPIO14)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	if(pin_mask & PIN_GPIO15)
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
}

void ICACHE_FLASH_ATTR dhgpio_open_drain(unsigned int pin_mask_set_od, unsigned int pin_mask_unset_od) {
	int i;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		unsigned int pin = 1 << i;
		if(pin & DHGPIO_SUITABLE_PINS == 0)
			continue;
		if(pin & pin_mask_set_od) {
			const unsigned int reg = GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(i))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE);
			GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(i)), reg);
		} else if (pin & pin_mask_unset_od) {
			const unsigned int reg = GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(i))) | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE);
			GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(i)), reg);
		}
	}
}

void ICACHE_FLASH_ATTR dhgpio_pull(unsigned int pollup_mask, unsigned int nopoll_mask) {
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
}

int ICACHE_FLASH_ATTR dhgpio_write(unsigned int set_mask, unsigned int unset_mask) {
	const unsigned int pins = set_mask | unset_mask;
	if((pins | DHGPIO_SUITABLE_PINS) != DHGPIO_SUITABLE_PINS)
		return 0;
	dhdebug("GPIO set high at 0x%X, set low at 0x%X", set_mask, unset_mask);
	dhgpio_prepare_pins(pins, 1);
	dhgpio_open_drain(0, pins);
	dhgpio_pull(0, pins);
	gpio_output_set(set_mask, unset_mask, pins, 0);
	return 1;
}

int ICACHE_FLASH_ATTR dhgpio_init(unsigned int init_mask, unsigned int pollup_mask, unsigned int nopoll_mask) {
	if(pollup_mask & nopoll_mask)
		return 0;
	if( (init_mask | pollup_mask | nopoll_mask | DHGPIO_SUITABLE_PINS) != DHGPIO_SUITABLE_PINS)
		return 0;
	// init
	dhgpio_prepare_pins(init_mask | pollup_mask | nopoll_mask, 1);
	gpio_output_set(0, 0, 0, init_mask | pollup_mask | nopoll_mask);
	// pullup
	dhgpio_pull(pollup_mask, nopoll_mask);
	return 1;
}

unsigned int ICACHE_FLASH_ATTR dhgpio_read() {
	return gpio_input_get() & DHGPIO_SUITABLE_PINS;
}

LOCAL ICACHE_FLASH_ATTR void gpio_timeout(void *arg) {
	dhgpio_int_timeout(mTriggeredIntrPins);
	mTriggeredIntrPins = 0;
	mGPIOTimerArmed = 0;
}

LOCAL ICACHE_FLASH_ATTR void gpio_intr(void *arg) {
	const unsigned int gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
	mTriggeredIntrPins |= gpio_status;
	if(mGPIOTimerArmed)
		return;
	os_timer_disarm(&mGPIOTimer);
	if(mGPIOTimerTimeout) {
		mGPIOTimerArmed = 1;
		os_timer_setfn(&mGPIOTimer, (os_timer_func_t *)gpio_timeout, NULL);
		os_timer_arm(&mGPIOTimer, mGPIOTimerTimeout, 0);
	} else {
		gpio_timeout(0);
	}
}

int ICACHE_FLASH_ATTR dhgpio_int(unsigned int disable_mask, unsigned int rising_mask, unsigned int falling_mask, unsigned int both_mask, unsigned int timeout) {
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
	if( (disable_mask | rising_mask | falling_mask | both_mask | DHGPIO_SUITABLE_PINS) != DHGPIO_SUITABLE_PINS)
		return 0;
	int i;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
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
	}
	mGPIOTimerTimeout = timeout;
	ETS_GPIO_INTR_ATTACH(gpio_intr, NULL);
	ETS_GPIO_INTR_ENABLE();
	return 1;
}

unsigned int ICACHE_FLASH_ATTR dhgpio_get_timeout() {
	return mGPIOTimerTimeout;
}
