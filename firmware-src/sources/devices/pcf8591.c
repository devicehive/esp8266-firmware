/*
 * pcf8591.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "pcf8591.h"
#include "dhi2c.h"
#include "dhdebug.h"
#include "dhutils.h"

static int mAddress = PCF8591_DEFAULT_ADDRESS;
static float mVoltage = 3.3f;

DHI2C_STATUS ICACHE_FLASH_ATTR pcf8591_read(int sda, int scl, float *values) {
	char buf[4];
	DHI2C_STATUS status;
	if(sda != PCF8591_NO_PIN && scl != PCF8591_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("pcf8591: failed to set up pins");
			return status;
		}
	}

	buf[0] = 0x45;  // Config
	if((status = dhi2c_write(mAddress, buf, 1, 1)) != DHI2C_OK) {
		dhdebug("pcf8591: failed to write");
		return status;
	}

	//dummy read to make sure all values was updated
	if((status = dhi2c_read(mAddress, buf, 4)) != DHI2C_OK) {
		dhdebug("pcf8591: failed to write");
		return status;
	}
	os_delay_us(250);
	if((status = dhi2c_read(mAddress, buf, 4)) != DHI2C_OK) {
		dhdebug("pcf8591: failed to write");
		return status;
	}
	int i;
	for(i = 0; i < sizeof(buf); i++) {
		values[i] = mVoltage * ((float)buf[i]) / 255.0f;
	}
	return DHI2C_OK;
}

DHI2C_STATUS ICACHE_FLASH_ATTR pcf8591_write(int sda, int scl, float value) {
	char buf[3];
	DHI2C_STATUS status;
	if(value < 0.0f || value > mVoltage)
		return DHI2C_WRONG_PARAMETERS;
	if(sda != PCF8591_NO_PIN && scl != PCF8591_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("pcf8591: failed to set up pins");
			return status;
		}
	}

	char v = (value / mVoltage * 255.0f);

	buf[0] = 0x44;  // Config
	buf[1] = v;
	if((status = dhi2c_write(mAddress, buf, 2, 1)) != DHI2C_OK) {
		dhdebug("pcf8591: failed to write");
		return status;
	}
	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR pcf8591_set_address(int address) {
	mAddress = address;
}

DHI2C_STATUS ICACHE_FLASH_ATTR pcf8591_set_vref(float voltage) {
	if(mVoltage <= 0.0f)
		return DHI2C_WRONG_PARAMETERS;
	mVoltage = voltage;
	return DHI2C_OK;
}
