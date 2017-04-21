/*
 * si7021.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "si7021.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

static int mAddress = SI7021_DEFAULT_ADDRESS;

DH_I2C_Status ICACHE_FLASH_ATTR si7021_read(int sda, int scl, float *humidity, float *temperature) {
	char buf[2];
	DH_I2C_Status status;
	if(sda != SI7021_NO_PIN && scl != SI7021_NO_PIN) {
		if((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("si7021: failed to set up pins");
			return status;
		}
	}

	buf[0] = 0xE5; // read humidity, hold master
	if((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("si7021: failed to write get humidity");
		return status;
	}
	if((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
		dhdebug("si7021: failed to read humidity");
		return status;
	}
	*humidity = ((float)unsignedInt16be(buf, 0)) * 125.0f / 65536.0f - 6.0f;

	if(temperature) {
		buf[0] = 0xE0; // read temperature from humidity measurement
		if((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
			dhdebug("si7021: failed to write get temperature");
			return status;
		}
		if((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
			dhdebug("si7021: failed to read temperature");
			return status;
		}
		*temperature = ((float)unsignedInt16be(buf, 0)) * 175.72f / 65536.0f - 46.85f;
	}

	return DH_I2C_OK;
}

void ICACHE_FLASH_ATTR si7021_set_address(int address) {
	mAddress = address;
}
