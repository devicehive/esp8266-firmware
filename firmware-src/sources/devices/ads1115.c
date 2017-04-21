/*
 * ads1115.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "ads1115.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

static int mAddress = ADS1115_DEFAULT_ADDRESS;

DH_I2C_Status ICACHE_FLASH_ATTR ads1115_read(int sda, int scl, float *values) {
	char buf[3];
	DH_I2C_Status status;
	int channel;
	if(sda != ADS1115_NO_PIN && scl != ADS1115_NO_PIN) {
		if((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("ads1115: failed to set up pins");
			return status;
		}
	}

	for(channel = 0; channel < 4; channel++) {
		buf[0] = 0x01; // Config register
		buf[1] = ((0b100 | channel) << 4) | 0x83; // single shot mode, +-4.096V, measure between input and gnd.
		buf[2] = 0x80; // 128 samples per second, comparator is disabled.
		if((status = dh_i2c_write(mAddress, buf, 3, 1)) != DH_I2C_OK) {
			dhdebug("ads1115: failed to write config");
			return status;
		}
		do {
			if((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
				dhdebug("ads1115: failed to write get config");
				return status;
			}
			if((status = dh_i2c_read(mAddress, &buf[1], 2)) != DH_I2C_OK) {
				dhdebug("ads1115: failed to read config");
				return status;
			}
		} while((buf[1] & 0x80) == 0); // while conversion is in progress

		buf[0] = 0x00; // get conversation register
		if((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
			dhdebug("ads1115: failed to write get conversation register");
			return status;
		}
		if((status = dh_i2c_read(mAddress, &buf[1], 2)) != DH_I2C_OK) {
			dhdebug("ads1115: failed to read coefficients");
			return status;
		}
		*values = (float)signedInt16be(buf, 1) / 32768.0f * 4.096f;
		values++;
	}

	return DH_I2C_OK;
}

void ICACHE_FLASH_ATTR ads1115_set_address(int address) {
	mAddress = address;
}
