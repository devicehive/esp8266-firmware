/**
 * @file
 * @brief Software implementation of onewire interface for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "DH/onewire.h"
#include "DH/adc.h"
#include "dhdebug.h"
#include "user_config.h"

#include <eagle_soc.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include <ets_sys.h>
#include <user_interface.h>
#include <ets_forward.h>

// module variables
static unsigned int mOneWirePin = 0;
static DHGpioPinMask mIntPins = 0;
static unsigned int mIntErrorCount = 0;
static unsigned int mWaitSearchPins = 0;
static os_timer_t mIntTimer;

#define ONEWIRE_MAX_INT_SEARCH_ATTEMPS  5
#define ONEWIRE_MAX_INT_DELAY_MS        20
#define ONEWIRE_RESET_LENGHT_US         640


/**
 * @brief Lock interruptions.
 */
static void ICACHE_FLASH_ATTR lock_int(void)
{
	if (mIntPins)
		dh_gpio_subscribe_extra_int(mIntPins, 0, 0, 0);
}


/**
 * @brief Unlock interruptions.
 */
static void ICACHE_FLASH_ATTR unlock_int(void)
{
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, mIntPins);
	dh_gpio_subscribe_extra_int(0, 0, mIntPins, 0);
}


/*
 * dh_onewire_set_pin() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_set_pin(unsigned int pin)
{
	if (!(DH_GPIO_PIN(pin) & DH_GPIO_SUITABLE_PINS))
		return -1; // unsuitable pin

	mOneWirePin = pin;
	return 0; // OK
}


/*
 * dh_onewire_get_pin() implementation.
 */
unsigned int ICACHE_FLASH_ATTR dh_onewire_get_pin(void)
{
	return mOneWirePin;
}


/**
 * @brief Reset onewire bus.
 * @return Non-zero if device is presented. Zero otherwise.
 */
int ICACHE_FLASH_ATTR dh_onewire_reset(DHGpioPinMask pin_mask, int reset_time_us, int exit_on_presence)
{
	system_soft_wdt_feed();
	const int pinstate = (gpio_input_get() & pin_mask) !=0;
	dh_gpio_open_drain(pin_mask, 0);
	dh_gpio_pull_up(pin_mask, 0);
	dh_gpio_prepare_pins(pin_mask, 1);
	if (!pinstate) {
		gpio_output_set(pin_mask, 0, pin_mask, 0);
		os_delay_us(500);
	}
	if (!(gpio_input_get() & pin_mask))
		return 0;

	// send RESET
	gpio_output_set(0, pin_mask, pin_mask, 0);
	os_delay_us(reset_time_us);
	gpio_output_set(pin_mask, 0, pin_mask, 0);

	// check RESPONSE
	int i, presence = 0;
	for (i = 0; i < 480; i++) { // wait at least 480 usec
		if (!presence && i < 240) {
			if (i > 15) {
				if (!(gpio_input_get() & pin_mask))
					presence = 1;
			}
		} else if(exit_on_presence) {
			if (presence) {
				if (gpio_input_get() & pin_mask)
					return 1;
			} else {
				return 0;
			}
		}
		os_delay_us(1);
	}

	return presence;
}


/**
 * @brief Do 1 bit communication.
 */
static int ICACHE_FLASH_ATTR onewire_act_bit(int bit, DHGpioPinMask pin_mask)
{
	// time slot start
	char res = 0;
	ETS_INTR_LOCK();
	gpio_output_set(0, pin_mask, pin_mask, 0);
	os_delay_us(5);

	if (bit) {
		gpio_output_set(pin_mask, 0, pin_mask, 0);
		os_delay_us(5);
		if (gpio_input_get() & pin_mask)
			res = bit;
		ETS_INTR_UNLOCK();
		os_delay_us(80);
	} else {
		os_delay_us(85);
		gpio_output_set(pin_mask, 0, pin_mask, 0);
		ETS_INTR_UNLOCK();
	}

	os_delay_us(5); // pause between time slots
	return res;
}


/**
 * @brief Do 1 byte communication.
 */
static int ICACHE_FLASH_ATTR onewire_act_byte(int byte, DHGpioPinMask pin_mask)
{
	system_soft_wdt_feed();

	int i, res = 0;
	for (i = 0; i < 8; i++) // lsb-first
		res |= onewire_act_bit(byte & BIT(i), pin_mask);

	return res;
}


/*
 * dh_onewire_write() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_write(const void *buf_, size_t len)
{
	lock_int();

	const DHGpioPinMask pin_mask = DH_GPIO_PIN(mOneWirePin);
	int present = dh_onewire_reset(pin_mask, ONEWIRE_RESET_LENGHT_US, 0);
	if (!present) {
		unlock_int();
		return -1; // failed
	}

	const uint8_t *buf = (const uint8_t*)buf_;
	while (len--)
		onewire_act_byte(*buf++, pin_mask);

	unlock_int();
	return 0; // OK
}


/*
 * dh_onewire_read() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_read(void *buf_, size_t len) {
	lock_int();

	const DHGpioPinMask pin_mask = DH_GPIO_PIN(mOneWirePin);
	uint8_t *buf = (uint8_t*)buf_;
	while (len--)
		*buf++ = onewire_act_byte(0xFF, pin_mask);

	unlock_int();
	return 0; // OK
}


/**
 * @brief Check onewire CRC.
 * @return Zero on success.
 */
static int ICACHE_FLASH_ATTR dh_onewire_check_crc(const uint8_t data[8])
{
	int seed = 0;

	int i, j;
	for (i = 0; i < 8; i++) {
		int b = data[i];
		for (j = 0; j < 8; j++) {
			if((seed ^ b) & 0x01)
				seed = (seed >> 1) ^ 0x8C;
			else
				seed >>= 1;
			b >>= 1;
		}
	}

	return seed;
}


/*
 * dh_onewire_search() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_search(void *buf_, size_t *len, int command, unsigned int pin)
{
	uint8_t address[8] = {0};
	if (*len < sizeof(address))
		return -1; // too few space in buffer

	const DHGpioPinMask pin_mask = DH_GPIO_PIN(pin);
	int lastAmbiguity = 8*sizeof(address);
	char *buf = (char*)buf_;
	size_t copyied = 0;

	lock_int();

	do {
		if (!dh_onewire_reset(pin_mask, ONEWIRE_RESET_LENGHT_US, 0)) {
			unlock_int();
			if (lastAmbiguity == 8*sizeof(address)) {
				*len = 0; // devices are not connected
				return 0;
			}
			return -1; // something wrong
		}

		onewire_act_byte(command, pin_mask);
		int i, ambiguity = -1;
		for (i = 0; i < 8*sizeof(address); i++) {
			if (!(gpio_input_get() & pin_mask)) {
				unlock_int();
				return -1; // lost
			}

			const int byte = i/8;
			const int bit = BIT(i%8);

			const int bit1 = onewire_act_bit(0x1, pin_mask);
			const int bit2 = onewire_act_bit(0x1, pin_mask);
			if (bit1 && bit2) {
				unlock_int();
				if (i == 0 && lastAmbiguity == 8*sizeof(address)) {
					*len = 0; // first interaction, no devices found
					return 0; // OK, no devices
				}
				// dhdebug("Onewire error while search at %u bit", i);
				return -1; // something wrong
			} else if(bit1 && !bit2) {
				address[byte] |= bit;
				onewire_act_bit(0x1, pin_mask);
			} else if(!bit1 && bit2) {
				address[byte] &= ~bit;
				onewire_act_bit(0x0, pin_mask);
			} else {
				if (i < lastAmbiguity) {
					address[byte] &= ~bit;
					onewire_act_bit(0x0, pin_mask);
					ambiguity = i;
				} else {
					address[byte] |= bit;
					onewire_act_bit(0x1, pin_mask);
				}
			}
		}
		lastAmbiguity = ambiguity;

		if (0 == dh_onewire_check_crc(address)) {
			os_memcpy(buf, address, sizeof(address));
			buf += sizeof(address);
			copyied += sizeof(address);
			if (copyied +  sizeof(address) > *len)
				break; // no more space
		} else {
			unlock_int();
			return -1; // bad CRC
		}
	} while (lastAmbiguity >= 0);

	*len = copyied;
	unlock_int();
	return 0; // OK
}


/**
 * @brief Timer callback.
 */
static void ICACHE_FLASH_ATTR onewire_int_search(void *arg)
{
	int i;
	for (i = 0; i < DH_GPIO_PIN_COUNT; i++) {
		const DHGpioPinMask pin = DH_GPIO_PIN(i);
		if (pin & mWaitSearchPins) {
			uint8_t buf[INTERFACES_BUF_SIZE];
			size_t len = sizeof(buf);
			int res = dh_onewire_search(buf, &len, 0xF0, i);
			if (len == 0)
				res = 1; // not found
			if (0 == res)
				dh_onewire_search_result(i, buf, len);
			else
				mIntErrorCount++;
			if (0 == res || mIntErrorCount > ONEWIRE_MAX_INT_SEARCH_ATTEMPS) {
				mIntErrorCount = 0;
				mWaitSearchPins &= ~pin;
			}

			// re-arm timer to avoid long operating in timer interruption.
			if (mWaitSearchPins) {
				os_timer_disarm(&mIntTimer);
				os_timer_setfn(&mIntTimer, onewire_int_search, NULL);
				os_timer_arm(&mIntTimer, ONEWIRE_MAX_INT_DELAY_MS, 0);
			}
			break;
		}
	}
}


/*
 * dh_gpio_extra_int_cb() implementation.
 */
void ICACHE_FLASH_ATTR dh_gpio_extra_int_cb(DHGpioPinMask caused_pins)
{
	os_timer_disarm(&mIntTimer);
	mWaitSearchPins |= caused_pins;
	os_timer_setfn(&mIntTimer, onewire_int_search, NULL);
	os_timer_arm(&mIntTimer, ONEWIRE_MAX_INT_DELAY_MS, 0);
}


/*
 * dh_onewire_int() implementation.
 */
int ICACHE_FLASH_ATTR dh_onewire_int(DHGpioPinMask search_pins, DHGpioPinMask disable_pins)
{
	int res = dh_gpio_subscribe_extra_int(disable_pins, 0, search_pins, 0);
	if (!!res)
		return res; // failed to subscribe

	mIntPins |= search_pins;
	mIntPins &= ~disable_pins;
	return 0; // OK
}
