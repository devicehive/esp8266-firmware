/**
 * @file
 * @brief GPIO hardware access layer for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "DH/gpio.h"
#include "DH/pwm.h"
#include "dhmem.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <ets_forward.h>

// module variables
static os_timer_t mTimer;
static unsigned int mTimeoutMs = 250;
static unsigned char mTimerArmed = 0;
static DHGpioPinMask mTriggeredIntrPins = 0;
static DHGpioPinMask mExternalIntPins = 0;


static void timeout_cb(void *arg);
static void int_cb(void *arg);


/*
 * dh_gpio_init() implementation.
 */
void ICACHE_FLASH_ATTR dh_gpio_init(void)
{
	ETS_GPIO_INTR_ATTACH(int_cb, NULL);
	ETS_GPIO_INTR_ENABLE();
}


/*
 * dh_gpio_prepare_pins() implementation.
 */
void ICACHE_FLASH_ATTR dh_gpio_prepare_pins(DHGpioPinMask pins, int disable_pwm)
{
	if (disable_pwm)
		dh_pwm_disable(pins);

	// only suitable pins are checked: GPIO0..GPIO5
	if (pins & DH_GPIO_PIN(0))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	if (pins & DH_GPIO_PIN(1))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
	if (pins & DH_GPIO_PIN(2))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	if (pins & DH_GPIO_PIN(3))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
	if (pins & DH_GPIO_PIN(4))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	if (pins & DH_GPIO_PIN(5))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);

	// only suitable pins are checked: GPIO12..GPIO15
	if (pins & DH_GPIO_PIN(12))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
	if (pins & DH_GPIO_PIN(13))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
	if (pins & DH_GPIO_PIN(14))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	if (pins & DH_GPIO_PIN(15))
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
}


/*
 * dh_gpio_open_drain() implementation.
 */
void ICACHE_FLASH_ATTR dh_gpio_open_drain(DHGpioPinMask pins_enable, DHGpioPinMask pins_disable)
{
	int i;
	for (i = 0; i < DH_GPIO_PIN_COUNT; ++i) {
		const DHGpioPinMask pin = DH_GPIO_PIN(i);
		if (!(pin & DH_GPIO_SUITABLE_PINS))
			continue; // skip unsuitable pins

		else if (pin & pins_enable) {
			const unsigned int reg = GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(i)))
			                       | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE);

			GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(i)), reg);
		} else if (pin & pins_disable) {
			const unsigned int reg = GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(i)))
			                       | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE);
			GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(i)), reg);
		}
	}
}


/*
 * dh_gpio_pull() implementation.
 */
void ICACHE_FLASH_ATTR dh_gpio_pull_up(DHGpioPinMask pins_enable, DHGpioPinMask pins_disable)
{
	// enable suitable GPIO pins
	if (pins_enable & DH_GPIO_PIN(0))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
	if (pins_enable & DH_GPIO_PIN(1))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_U0TXD_U);
	if (pins_enable & DH_GPIO_PIN(2))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
	if (pins_enable & DH_GPIO_PIN(3))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_U0RXD_U);
	if (pins_enable & DH_GPIO_PIN(4))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);
	if (pins_enable & DH_GPIO_PIN(5))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
	if (pins_enable & DH_GPIO_PIN(12))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_MTDI_U);
	if (pins_enable & DH_GPIO_PIN(13))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_MTCK_U);
	if (pins_enable & DH_GPIO_PIN(14))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_MTMS_U);
	if (pins_enable & DH_GPIO_PIN(15))
		PIN_PULLUP_EN(PERIPHS_IO_MUX_MTDO_U);

	// disable suitable GPIO pins
	if (pins_disable & DH_GPIO_PIN(0))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO0_U);
	if (pins_disable & DH_GPIO_PIN(1))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
	if (pins_disable & DH_GPIO_PIN(2))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO2_U);
	if (pins_disable & DH_GPIO_PIN(3))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0RXD_U);
	if (pins_disable & DH_GPIO_PIN(4))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO4_U);
	if (pins_disable & DH_GPIO_PIN(5))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO5_U);
	if (pins_disable & DH_GPIO_PIN(12))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDI_U);
	if (pins_disable & DH_GPIO_PIN(13))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTCK_U);
	if (pins_disable & DH_GPIO_PIN(14))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTMS_U);
	if (pins_disable & DH_GPIO_PIN(15))
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_MTDO_U);
}


/*
 * dh_gpio_write() implementation.
 */
int ICACHE_FLASH_ATTR dh_gpio_write(DHGpioPinMask pins_set, DHGpioPinMask pins_unset)
{
	const DHGpioPinMask pins = pins_set | pins_unset;
	if (pins & ~DH_GPIO_SUITABLE_PINS)
		return -1; // unsuitable pins

	dh_gpio_prepare_pins(pins, true/*disable_pwm*/);
	dh_gpio_open_drain(0, pins);
	dh_gpio_pull_up(0, pins);
	gpio_output_set(pins_set, pins_unset, pins, 0);
	return 0; // OK
}


/*
 * dh_gpio_input() implementation.
 */
int ICACHE_FLASH_ATTR dh_gpio_input(DHGpioPinMask pins_init,
                                    DHGpioPinMask pins_pullup,
                                    DHGpioPinMask pins_nopull)
{
	const DHGpioPinMask pins = pins_init | pins_pullup | pins_nopull;
	if (pins & ~DH_GPIO_SUITABLE_PINS)
		return -1; // unsuitable pins
	if (pins_pullup & pins_nopull)
		return -2; // bad input parameters

	// initialize
	dh_gpio_prepare_pins(pins, true/*disable_pwm*/);
	gpio_output_set(0, 0, 0, pins);

	// pull-up
	dh_gpio_pull_up(pins_pullup, pins_nopull);

	return 0; // OK
}


/*
 * dh_gpio_read() implementation.
 */
DHGpioPinMask ICACHE_FLASH_ATTR dh_gpio_read(void)
{
	return gpio_input_get() & DH_GPIO_SUITABLE_PINS;
}


/**
 * @brief Timeout callback.
 */
static void ICACHE_FLASH_ATTR timeout_cb(void *arg)
{
	// call external handler
	dh_gpio_int_cb(mTriggeredIntrPins);

	mTriggeredIntrPins = 0;
	mTimerArmed = 0;
}

/**
 * @brief Interruption handler.
 */
static void int_cb(void *arg)
{
	uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
	if (gpio_status & mExternalIntPins) {
		dh_gpio_extra_int_cb(gpio_status & mExternalIntPins);
		gpio_status &= ~mExternalIntPins;
		if (!gpio_status)
			return;
	}

	mTriggeredIntrPins |= gpio_status;
	if (mTimerArmed)
		return;

	os_timer_disarm(&mTimer);
	if (mTimeoutMs) {
		mTimerArmed = 1;
		os_timer_setfn(&mTimer, timeout_cb, NULL);
		os_timer_arm(&mTimer, mTimeoutMs, 0);
	} else {
		timeout_cb(0);
	}
}


/**
 * @brief Enable GPIO interruption.
 * @return Zero on success.
 */
static int ICACHE_FLASH_ATTR dh_gpio_set_int(DHGpioPinMask pins_disable,
                                             DHGpioPinMask pins_rising,
                                             DHGpioPinMask pins_falling,
                                             DHGpioPinMask pins_both)
{
	DHGpioPinMask pins = pins_disable;
	if (pins & pins_rising)
		return -2; // bad input parameters
	pins |= pins_rising;
	if (pins & pins_falling)
		return -2; // bad input parameters
	pins |= pins_falling;
	if (pins & pins_both)
		return -2; // bad input parameters
	pins |= pins_both;
	if (pins & ~DH_GPIO_SUITABLE_PINS)
		return -1; // unsuitable pins

	int i;
	for (i = 0; i < DH_GPIO_PIN_COUNT; ++i) {
		const DHGpioPinMask pin = DH_GPIO_PIN(i);
		if (!(pin & DH_GPIO_SUITABLE_PINS))
			continue; // skip unsuitable pins

		else if(pin & pins_disable)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_DISABLE);
		else if(pin & pins_falling)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_POSEDGE);
		else if(pin & pins_rising)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_NEGEDGE);
		else if(pin & pins_both)
			gpio_pin_intr_state_set(GPIO_ID_PIN(i), GPIO_PIN_INTR_ANYEDGE);
	}

	return 0; // OK
}


/*
 * dh_gpio_subscribe_int() implementation.
 */
int ICACHE_FLASH_ATTR dh_gpio_subscribe_int(DHGpioPinMask pins_disable,
		                                    DHGpioPinMask pins_rising,
		                                    DHGpioPinMask pins_falling,
		                                    DHGpioPinMask pins_both,
		                                    unsigned int timeout_ms)
{
	const DHGpioPinMask pins = pins_disable | pins_rising | pins_falling | pins_both;
	if (!!dh_gpio_subscribe_extra_int(pins, 0, 0, 0))
		return -1; // failed to disable all extra interruptions

	// subscribe
	int r = dh_gpio_set_int(pins_disable,
	                        pins_rising,
	                        pins_falling,
	                        pins_both);
	if (0 == r) {
		// OK, save timeout...
		mTimeoutMs = timeout_ms;
	}

	return r;
}


/*
 * dh_gpio_get_timeout() implementation.
 */
unsigned int ICACHE_FLASH_ATTR dh_gpio_get_timeout(void)
{
	return mTimeoutMs;
}


/*
 * dh_gpio_subscribe_extra_int() implementation.
 */
int ICACHE_FLASH_ATTR dh_gpio_subscribe_extra_int(DHGpioPinMask pins_disable,
                                                  DHGpioPinMask pins_rising,
                                                  DHGpioPinMask pins_falling,
                                                  DHGpioPinMask pins_both)
{
	pins_disable &= mExternalIntPins;
	int r = dh_gpio_set_int(pins_disable,
	                        pins_rising,
	                        pins_falling,
	                        pins_both);
	if (0 == r) {
		// OK, update corresponding register
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, pins_disable);
		mExternalIntPins |= pins_rising | pins_falling | pins_both;
		mExternalIntPins &= ~pins_disable;
	}

	return r;
}
