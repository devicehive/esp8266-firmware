/*
 * dhi2c.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: I2C module
 *
 */
#include "dhi2c.h"
#include "dhgpio.h"
#include "user_config.h"

#include <c_types.h>
#include <osapi.h>
#include <gpio.h>
#include <user_interface.h>
#include <ets_forward.h>

#define I2C_DELAY_US 5
#define I2C_ERROR_TIMEOUT_US 50000

LOCAL unsigned int mSDAPin = (1 << 0);
LOCAL unsigned int mSCLPin = (1 << 2);

DHI2C_STATUS ICACHE_FLASH_ATTR dhi2c_init(unsigned int sda_pin, unsigned int scl_pin) {
	const unsigned int SDA = (1 << sda_pin);
	const unsigned int SCL = (1 << scl_pin);
	if((SDA & DHGPIO_SUITABLE_PINS) == 0 || (SCL & DHGPIO_SUITABLE_PINS) == 0 || SDA == SCL)
		return DHI2C_WRONG_PARAMETERS;

	mSDAPin = SDA;
	mSCLPin = SCL;
	dhi2c_reinit();
	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR dhi2c_reinit(void) {
	dhgpio_open_drain(mSDAPin | mSCLPin, 0);
	dhgpio_prepare_pins(mSDAPin | mSCLPin, 1);
	dhgpio_pull(mSDAPin | mSCLPin, 0);
	gpio_output_set(mSDAPin | mSCLPin, 0, mSDAPin | mSCLPin, 0);
}

LOCAL DHI2C_STATUS ICACHE_FLASH_ATTR dhi2c_set_pin(unsigned int pin_mask, int val) {
	if(val) {
		gpio_output_set(pin_mask, 0, pin_mask, 0);
		val = pin_mask;
	} else {
		gpio_output_set(0, pin_mask, pin_mask, 0);
	}
	os_delay_us(I2C_DELAY_US);
	int i;
	for(i = 0; i < I2C_ERROR_TIMEOUT_US; i++) {
		const unsigned int c = gpio_input_get();
		if( (c & pin_mask) == val)
			return DHI2C_OK;
		os_delay_us(1);
	}
	return DHI2C_BUS_BUSY;
}

LOCAL DHI2C_STATUS ICACHE_FLASH_ATTR dhi2c_start(void) {
	DHI2C_STATUS res;
	res = dhi2c_set_pin(mSDAPin, 1);
	if(res != DHI2C_OK)
		return res;
	res = dhi2c_set_pin(mSCLPin, 1);
	if(res != DHI2C_OK)
		return res;
	res = dhi2c_set_pin(mSDAPin, 0);
	if(res != DHI2C_OK)
		return res;
	res = dhi2c_set_pin(mSCLPin, 0);
	if(res != DHI2C_OK)
		return res;
	return DHI2C_OK;
}

LOCAL DHI2C_STATUS ICACHE_FLASH_ATTR dhi2c_stop(void) {
	DHI2C_STATUS res;
	res = dhi2c_set_pin(mSDAPin, 0);
	if(res != DHI2C_OK)
		return res;
	res = dhi2c_set_pin(mSCLPin, 1);
	if(res != DHI2C_OK)
		return res;
	res = dhi2c_set_pin(mSDAPin, 1);
	if(res != DHI2C_OK)
		return res;
	return DHI2C_OK;
}

LOCAL DHI2C_STATUS ICACHE_FLASH_ATTR dhi2c_writebyte(unsigned char byte) {
	DHI2C_STATUS res;
	int i;
	for (i = 7; i >= 0; i--) {
		const unsigned int bit = ((1 << i) & byte);
		res =  dhi2c_set_pin(mSDAPin, bit);
		if(res != DHI2C_OK)
			return res;
		res = dhi2c_set_pin(mSCLPin, 1);
		if(res != DHI2C_OK)
			return res;
		os_delay_us(I2C_DELAY_US);
		res = dhi2c_set_pin(mSCLPin, 0);
		if(res != DHI2C_OK)
			return res;
	}
	// check ACK
	gpio_output_set(mSDAPin, 0, mSDAPin, 0);
	os_delay_us(I2C_DELAY_US);
	res = dhi2c_set_pin(mSCLPin, 1);
	if(res != DHI2C_OK)
		return res;
	os_delay_us(I2C_DELAY_US);
	const unsigned int ackbit = (gpio_input_get() & mSDAPin) ? 1 : 0;
	res = dhi2c_set_pin(mSCLPin, 0);
	if(res != DHI2C_OK)
		return res;
	if(ackbit)
		return DHI2C_NOACK;
	return DHI2C_OK;
}

DHI2C_STATUS ICACHE_FLASH_ATTR dhi2c_readbyte(unsigned char *byte, int ackbit) {
	DHI2C_STATUS res;
	gpio_output_set(mSDAPin, 0, mSDAPin, 0);
	os_delay_us(I2C_DELAY_US);
	int i;
	unsigned char rb = 0;
	for (i = 7; i >= 0; i--) {
		res = dhi2c_set_pin(mSCLPin, 1);
		if(res != DHI2C_OK)
			return res;
		os_delay_us(I2C_DELAY_US);
		const unsigned int bit = (gpio_input_get() & mSDAPin) ? 1 :0;
		rb |= bit << i;
		res = dhi2c_set_pin(mSCLPin, 0);
		if(res != DHI2C_OK)
			return res;
	}
	// send ACK
	res =  dhi2c_set_pin(mSDAPin, ackbit);
	if(res != DHI2C_OK)
		return res;
	res = dhi2c_set_pin(mSCLPin, 1);
	if(res != DHI2C_OK)
		return res;
	os_delay_us(I2C_DELAY_US);
	res = dhi2c_set_pin(mSCLPin, 0);
	if(res != DHI2C_OK)
		return res;
	*byte = rb;
	return DHI2C_OK;
}

LOCAL DHI2C_STATUS ICACHE_FLASH_ATTR dhi2c_act(unsigned int address, char *buf, unsigned int len, int sendstop, int is_read) {
	if(len > INTERFACES_BUF_SIZE || address > 0xFF)
			return DHI2C_WRONG_PARAMETERS;
	DHI2C_STATUS res;
	res = dhi2c_start();
	if(res != DHI2C_OK)
		return res;
	if(is_read)
		res = dhi2c_writebyte(address | 0x1);
	else
		res = dhi2c_writebyte(address & 0xFE);
	if(res != DHI2C_OK) {
		dhi2c_stop();
		return res;
	}
	int i;
	for(i = 0; i < len; i++) {
		system_soft_wdt_feed();
		if(is_read)	{
			res = dhi2c_readbyte(&buf[i], (i + 1) == len);
		} else {
			res = dhi2c_writebyte(buf[i]);
		}
		if(res != DHI2C_OK) {
			dhi2c_stop();
			return res;
		}
	}
	if(sendstop) {
		res = dhi2c_stop();
		if(res != DHI2C_OK)
			return res;
	}
	return DHI2C_OK;
}

DHI2C_STATUS ICACHE_FLASH_ATTR dhi2c_write(unsigned int address, const char *buf, unsigned int len, int sendstop) {
	return dhi2c_act(address, (char *)buf, len, sendstop, 0);
}

DHI2C_STATUS ICACHE_FLASH_ATTR dhi2c_read(unsigned int address, char *buf, unsigned int len) {
	return dhi2c_act(address, buf, len, 1, 1);
}
