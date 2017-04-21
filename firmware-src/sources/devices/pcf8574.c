/*
 * pcf8574.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "pcf8574.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

static int mAddress = PCF8574_DEFAULT_ADDRESS;

DH_I2C_Status ICACHE_FLASH_ATTR pcf8574_read(int sda, int scl, unsigned int *pins) {
	char buf;
	DH_I2C_Status status;
	if(sda != PCF8574_NO_PIN && scl != PCF8574_NO_PIN) {
		if((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("pcf8574: failed to set up pins");
			return status;
		}
	}
	if((status = dh_i2c_read(mAddress, &buf, 1)) != DH_I2C_OK) {
		dhdebug("pcf8574: failed to read");
		return status;
	}

	*pins = (unsigned char)buf;
	return DH_I2C_OK;
}

DH_I2C_Status ICACHE_FLASH_ATTR pcf8574_write(int sda, int scl, unsigned int pins_to_set, unsigned int pins_to_clear) {
	char buf;
	DH_I2C_Status status;
	unsigned int current_state;
	if(pins_to_set & pins_to_clear)
		return DH_I2C_WRONG_PARAMETERS;
	if((status = pcf8574_read(sda, scl, &current_state)) != DH_I2C_OK) {
		return status;
	}
	buf = (char)((current_state | pins_to_set) & (~pins_to_clear) & PCF8574_SUITABLE_PINS);
	if((status = dh_i2c_write(mAddress, &buf, 1, 1)) != DH_I2C_OK) {
		dhdebug("pcf8574: failed to write");
		return status;
	}
	return DH_I2C_OK;
}

DH_I2C_Status ICACHE_FLASH_ATTR pcf8574_set(int sda, int scl, unsigned int pins) {
	char buf;
	DH_I2C_Status status;
	if(sda != PCF8574_NO_PIN && scl != PCF8574_NO_PIN) {
		if((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("pcf8574: failed to set up pins");
			return status;
		}
	}
	buf = pins & PCF8574_SUITABLE_PINS;
	if((status = dh_i2c_write(mAddress, &buf, 1, 1)) != DH_I2C_OK) {
		dhdebug("pcf8574: failed to write");
		return status;
	}
	return DH_I2C_OK;
}

void ICACHE_FLASH_ATTR pcf8574_set_address(int address) {
	mAddress = address;
}
