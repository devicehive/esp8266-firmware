/*
 * ina219.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "ina219.h"
#include "dhi2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>
#include <ets_forward.h>

static int mAddress = INA219_DEFAULT_ADDRESS;
static float mResistance = 0.1f;

DHI2C_STATUS ICACHE_FLASH_ATTR ina219_read(int sda, int scl, float *voltage, float *current, float *power) {
	char buf[12];
	DHI2C_STATUS status;
	if(sda != INA219_NO_PIN && scl != INA219_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("ina219: failed to set up pins");
			return status;
		}
	}

	uint16_t cal = 4096.0f / mResistance;
	buf[0] = 0x05;
	buf[1] = ((cal >> 8) & 0xFF);
	buf[2] = (cal & 0xFF);
	if((status = dhi2c_write(mAddress, buf, 3, 1)) != DHI2C_OK) {
		dhdebug("ina219: failed to write calibration data");
		return status;
	}

	os_delay_us(600);

	char i;
	for(i = 1; i < 5; i++) {
		if((status = dhi2c_write(mAddress, &i, 1, 0)) != DHI2C_OK) {
			dhdebug("ina219: failed to write address");
			return status;
		}
		if((status = dhi2c_read(mAddress, &buf[i * 2], 2)) != DHI2C_OK) {
			dhdebug("ina219: failed to read data");
			return status;
		}
	}
	//float shunt_voltage = ((float)signedInt16be(buf, 2)) / 100000.0f;
	*voltage = (((float)((unsigned int)buf[4]) * 32 + (buf[5] > 3))) / 8000.0f * 32.0f;
	*current = ((float)signedInt16be(buf, 8)) / 100000.0f;
	*power = ((float)unsignedInt16be(buf, 6)) / 100000.0f * 20.0f;

	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR ina219_set_address(int address) {
	mAddress = address;
}

DHI2C_STATUS ICACHE_FLASH_ATTR ina219_set_shunt(float resistance) {
	if(mResistance <= 0.0f)
		return DHI2C_WRONG_PARAMETERS;
	mResistance = resistance;
	return DHI2C_OK;
}
