/*
 * dhonewire.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Software onewire implementation
 *
 */

#include <c_types.h>
#include <eagle_soc.h>
#include <osapi.h>
#include <os_type.h>
#include "dhonewire.h"
#include "dhgpio.h"
#include "user_config.h"

LOCAL unsigned int mOneWirePin = 0;
LOCAL unsigned int mIntPins = 0;
LOCAL unsigned int mIntErrorCount = 0;
LOCAL unsigned int mWaitSearchPins = 0;
LOCAL os_timer_t mOWIntTimer;

#define ONEWIRE_MAX_INT_SEARCH_ATTEMPS 5
#define ONEWIRE_MAX_INT_DELAY_MS 20
#define ONEWIRE_RESET_LENGHT_US 640
#define ONEWIRE_DHT_RESET_LENGTH_US 25000
#define ONEWIRE_DHT_TIMEOUT_US 200

LOCAL ICACHE_FLASH_ATTR lock_int() {
	if(mIntPins)
		dhgpio_subscribe_extra_int(mIntPins, 0, 0, 0);
}

LOCAL ICACHE_FLASH_ATTR unlock_int() {
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, mIntPins);
	dhgpio_subscribe_extra_int(0, 0, mIntPins, 0);
}

int ICACHE_FLASH_ATTR dhonewire_set_pin(unsigned int pin) {
	if(((1 << pin) & DHGPIO_SUITABLE_PINS) == 0)
		return 0;
	mOneWirePin = pin;
	return 1;
}

int ICACHE_FLASH_ATTR dhonewire_get_pin() {
	return mOneWirePin;
}

LOCAL int ICACHE_FLASH_ATTR dhonewire_reset(unsigned int pin, unsigned int reset_length, int exit_on_presence) {
	system_soft_wdt_feed();
	const unsigned int pinstate = gpio_input_get() & pin;
	unsigned int i;
	unsigned int presence = 0;
	dhgpio_open_drain(pin, 0);
	dhgpio_pull(pin, 0);
	dhgpio_prepare_pins(pin, 1);
	if(pinstate == 0) {
		gpio_output_set(pin, 0, pin, 0);
		os_delay_us(500);
	}
	if(gpio_input_get() & pin == 0)
		return 0;
	// send RESET
	gpio_output_set(0, pin, pin, 0);
	os_delay_us(reset_length);
	gpio_output_set(pin, 0, pin, 0);
	// check RESPONCE
	for (i = 0; i < 480; i++) { // wait at least 480 msec
		if(presence == 0 && i < 240) {
			if(i > 15) {
				if((gpio_input_get() & pin) == 0)
					presence = 1;
			}
		} else if (exit_on_presence) {
			if(presence) {
				if(gpio_input_get() & pin)
					return 1;
			} else {
				return 0;
			}
		}
		os_delay_us(1);
	}
	return presence;
}

LOCAL char ICACHE_FLASH_ATTR dhonewire_act_bit(char bit, unsigned int pin) {
	// time slot start
	char res = 0;
	ETS_INTR_LOCK();
	gpio_output_set(0, pin, pin, 0);
	os_delay_us(5);
	if(bit) {
		gpio_output_set(pin, 0, pin, 0);
		os_delay_us(5);
		if(gpio_input_get() & pin)
			res |= bit;
		ETS_INTR_UNLOCK();
		os_delay_us(80);
	} else {
		os_delay_us(85);
		gpio_output_set(pin, 0, pin, 0);
		ETS_INTR_UNLOCK();
	}
	os_delay_us(5); // pause between time slots
	return res;
}

LOCAL char ICACHE_FLASH_ATTR dhonewire_act_byte(char byte, unsigned int pin) {
	system_soft_wdt_feed();
	unsigned int i = 0;
	char res = 0;
	for(i = 0; i < 8; i++)
		res |= dhonewire_act_bit(byte & (1 << i), pin);
	return res;
}

int ICACHE_FLASH_ATTR dhonewire_write(const char *buf, unsigned int len) {
	const unsigned int pin = (1 << mOneWirePin);
	lock_int();
	if(dhonewire_reset(pin, ONEWIRE_RESET_LENGHT_US, 0) == 0) {
		unlock_int();
		return 0;
	}
	while(len--)
		dhonewire_act_byte(*buf++, pin);
	unlock_int();
	return 1;
}

int ICACHE_FLASH_ATTR dhonewire_read(char *buf, unsigned int len) {
	const unsigned int pin = (1 << mOneWirePin);
	lock_int();
	while(len--)
		*buf++ = dhonewire_act_byte(0xFF, pin);
	unlock_int();
	return 1;
}

LOCAL unsigned int ICACHE_FLASH_ATTR dhonewire_check_crc(char *data) {
	unsigned int seed = 0;
	int i, j;
	for (i = 0; i < 8; i++) {
		char b = data[i];
		for (j = 0; j < 8; j++) {
			if ((seed ^ b) & 0x01)
				seed = (seed >> 1) ^ 0x8C;
			else
				seed >>= 1;
			b >>= 1;
		}
	}
	return seed;
}

int ICACHE_FLASH_ATTR dhonewire_search(char *buf, unsigned long *len, char command, unsigned int pin_number) {
	const unsigned int pin = (1 << pin_number);
	int i;
	char address[8] = {0};
	const unsigned int bitCount = sizeof(address) * 8;
	int lastAmbiguity = bitCount;
	int ambiguity;
	unsigned int copyied = 0;
	if(*len < 8)
		return 0;
	lock_int();
	do {
		int res = dhonewire_reset(pin, ONEWIRE_RESET_LENGHT_US, 0);
		if(res == 0) {
			unlock_int();
			if(lastAmbiguity == bitCount) {
				*len = 0; // devices are not connected
				return 1;
			}
			return 0;
		}
		dhonewire_act_byte(command, pin);
		ambiguity = -1;
		for(i = 0; i < bitCount; i++) {
			const int byte = i / 8;
			if(gpio_input_get() & pin == 0) {
				unlock_int();
				return 0;
			}
			const char bit = 1 << (i % 8);
			const char bit1 = dhonewire_act_bit(0x1, pin);
			const char bit2 = dhonewire_act_bit(0x1, pin);
			if(bit1 && bit2) {
				unlock_int();
				if(i == 0 && lastAmbiguity == bitCount) {
					*len = 0; // first interaction, no devices found
					return 1;
				}
				dhdebug("Onewire error while search at %u bit", i);
				return 0;
			} else if(bit1 && bit2 == 0) {
				address[byte] |= bit;
				dhonewire_act_bit(0x1, pin);
			} else if(bit1 == 0 && bit2) {
				address[byte] &= ~bit;
				dhonewire_act_bit(0x0, pin);
			} else {
				if(i < lastAmbiguity) {
					address[byte] &= ~bit;
					dhonewire_act_bit(0x0, pin);
					ambiguity = i;
				} else {
					address[byte] |= bit;
					dhonewire_act_bit(0x1, pin);
				}
			}
		}
		lastAmbiguity = ambiguity;
		if(dhonewire_check_crc(address) == 0) {
			os_memcpy(buf, address, sizeof(address));
			buf += sizeof(address);
			copyied += sizeof(address);
			if(copyied +  sizeof(address) > *len)
				break;
		} else {
			unlock_int();
			return 0;
		}
	} while (lastAmbiguity >= 0);
	*len = copyied;
	unlock_int();
	return 1;
}

LOCAL void ICACHE_FLASH_ATTR dhonewire_int_search(void *arg) {
	int i;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		const unsigned int pin = 1 << i;
		if(pin & mWaitSearchPins) {
			char buf[INTERFACES_BUF_SIZE];
			unsigned long len = sizeof(buf);
			unsigned int res = dhonewire_search(buf, &len, 0xF0, i);
			if(len == 0)
				res = 0;
			if(res)
				dhonewire_search_result(i, buf, len);
			else
				mIntErrorCount++;
			if(res || mIntErrorCount > ONEWIRE_MAX_INT_SEARCH_ATTEMPS) {
				mIntErrorCount = 0;
				mWaitSearchPins &= ~pin;
			}

			// rearm timer to avoid long operating in timer interruption.
			if(mWaitSearchPins) {
				os_timer_disarm(&mOWIntTimer);
				os_timer_setfn(&mOWIntTimer, (os_timer_func_t *)dhonewire_int_search, NULL);
				os_timer_arm(&mOWIntTimer, ONEWIRE_MAX_INT_DELAY_MS, 0);
			}
			break;
		}
	}
}

void ICACHE_FLASH_ATTR dhgpio_extra_int(unsigned int caused_pins) {
	os_timer_disarm(&mOWIntTimer);
	mWaitSearchPins |= caused_pins;
	os_timer_setfn(&mOWIntTimer, (os_timer_func_t *)dhonewire_int_search, NULL);
	os_timer_arm(&mOWIntTimer, ONEWIRE_MAX_INT_DELAY_MS, 0);
}

int ICACHE_FLASH_ATTR dhonewire_int(unsigned int search_pins, unsigned int disable_pins) {
	if(dhgpio_subscribe_extra_int(disable_pins, 0, search_pins, 0) == 0)
		return 0;
	mIntPins |= search_pins;
	mIntPins &= ~disable_pins;
	return 1;
}

LOCAL unsigned int ICACHE_FLASH_ATTR donewire_dht_measure_high(unsigned int pin) {
	unsigned int counter = 0;
	while((gpio_input_get() & pin) == 0) {
		if(counter > ONEWIRE_DHT_TIMEOUT_US)
			return 0;
		os_delay_us(1);
		counter++;
	}
	counter = 0;
	while((gpio_input_get() & pin)) {
		if(counter > ONEWIRE_DHT_TIMEOUT_US)
			return ONEWIRE_DHT_TIMEOUT_US;
		os_delay_us(1);
		counter++;
	}
	return counter;
}

int ICACHE_FLASH_ATTR dhonewire_dht_read(char *buf, unsigned int size) {
	const unsigned int pin = (1 << mOneWirePin);
	if(dhonewire_reset(pin, ONEWIRE_DHT_RESET_LENGTH_US, 1) == 0)
		return 0;
	ETS_INTR_LOCK();
	unsigned int i;
	if(donewire_dht_measure_high(pin) >= ONEWIRE_DHT_TIMEOUT_US) { // wait till response finish
		ETS_INTR_UNLOCK();
		return 0;
	}
	for(i = 0; i / 8 < size; i++) {
		unsigned int time = donewire_dht_measure_high(pin);
		if(time >= ONEWIRE_DHT_TIMEOUT_US || time <= 10) {
			break;
		} else {
			const char bit = 1 << (7 - i % 8);
			if(time > 25)
				buf[i / 8] |= bit;
			else
				buf[i / 8] &= ~bit;
		}
		system_soft_wdt_feed();
	}
	ETS_INTR_UNLOCK();
	return i / 8;
}
