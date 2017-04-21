/*
 * lm75.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "lm75.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

static int mAddress = LM75_DEFAULT_ADDRESS;

DH_I2C_Status ICACHE_FLASH_ATTR lm75_read(int sda, int scl, float *temperature) {
	char buf[2];
	int raw_temperature;
	DH_I2C_Status status;
	if(sda != LM75_NO_PIN && scl != LM75_NO_PIN) {
		if((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("lm75: failed to set up pins");
			return status;
		}
	}

    buf[0] = 0x0; // get temperature
    if((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("lm75: failed to write get temperature command");
		return status;
	}
	if((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
		dhdebug("lm75: failed to read temperature");
		return status;
	}
	raw_temperature = signedInt16be(buf, 0);

	*temperature = (float)raw_temperature / 256.0f;
	return DH_I2C_OK;
}

void ICACHE_FLASH_ATTR lm75_set_address(int address) {
	mAddress = address;
}
