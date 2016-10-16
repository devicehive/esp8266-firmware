/*
 * pcf8574.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "pcf8574.h"
#include "dhi2c.h"
#include "dhdebug.h"
#include "dhutils.h"

static int mAddress = PCF8574_DEFAULT_ADDRESS;

DHI2C_STATUS ICACHE_FLASH_ATTR pcf8574_read(int sda, int scl, unsigned int *pins) {
	char buf;
	DHI2C_STATUS status;
	if(sda != PCF8574_NO_PIN && scl != PCF8574_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("pcf8574: failed to set up pins");
			return status;
		}
	}
	if((status = dhi2c_read(mAddress, &buf, 1)) != DHI2C_OK) {
		dhdebug("pcf8574: failed to read");
		return status;
	}

	*pins = (unsigned char)buf;
	return DHI2C_OK;
}

DHI2C_STATUS ICACHE_FLASH_ATTR pcf8574_write(int sda, int scl, unsigned int pins_to_set, unsigned int pins_to_clear) {
    char buf;
    DHI2C_STATUS status;
    unsigned int current_state;
    if((status = pcf8574_read(sda, scl, &current_state)) != DHI2C_OK) {
    	return status;
    }
    buf = (char)(((current_state | pins_to_set) ^ pins_to_clear) & PCF8574_SUITABLE_PINS);
    if((status = dhi2c_write(mAddress, &buf, 1, 1)) != DHI2C_OK) {
        dhdebug("pcf8574: failed to write");
        return status;
    }

    return DHI2C_OK;
}

DHI2C_STATUS ICACHE_FLASH_ATTR pcf8574_set(int sda, int scl, unsigned int pins) {
	char buf;
	DHI2C_STATUS status;
	if(sda != PCF8574_NO_PIN && scl != PCF8574_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("pcf8574: failed to set up pins");
			return status;
		}
	}
	buf = pins & PCF8574_SUITABLE_PINS;
	if((status = dhi2c_write(mAddress, &buf, 1, 1)) != DHI2C_OK) {
		dhdebug("pcf8574: failed to write");
		return status;
	}
	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR pcf8574_set_address(int address) {
	mAddress = address;
}
