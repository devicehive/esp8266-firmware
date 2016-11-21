/*
 * mpu6050.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "mpu6050.h"
#include "dhi2c.h"
#include "dhdebug.h"
#include "dhutils.h"

static int mAddress = MPU6050_DEFAULT_ADDRESS;
#define EARTH_GRAVITY_ACCELERATION 9.80665f

DHI2C_STATUS ICACHE_FLASH_ATTR mpu6050_read(int sda, int scl,
		MPU6050_XYZ *acceleromter, MPU6050_XYZ *gyroscope, float *temparature)
{
	char buf[16];
	DHI2C_STATUS status;
	if(sda != MPU6050_NO_PIN && scl != MPU6050_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("mpu6050: failed to set up pins");
			return status;
		}
	}

	buf[0] = 0x6B; // power up
	buf[1] = 0; // no sleep bit
	if((status = dhi2c_write(mAddress, buf, 2, 1)) != DHI2C_OK) {
		dhdebug("mpu6050: failed to power up");
		return status;
	}

	buf[0] = 0x1C; // accelerometer configuration
	buf[1] = (1 << 4); // AFS_SEL = 2, range +-8 g
	if((status = dhi2c_write(mAddress, buf, 2, 1)) != DHI2C_OK) {
		dhdebug("mpu6050: failed to configure accelerometer");
		return status;
	}

	buf[0] = 0x1B; // gyroscope configuration
	buf[1] = (1 << 4); // FS_SEL = 2, range +-1000 dps
	if((status = dhi2c_write(mAddress, buf, 2, 1)) != DHI2C_OK) {
		dhdebug("mpu6050: failed to configure gyroscope");
		return status;
	}

	os_delay_us(50000);

	buf[0] = 0x3B; // get data
	if((status = dhi2c_write(mAddress, buf, 1, 0)) != DHI2C_OK) {
		dhdebug("mpu6050: failed to set read register");
		return status;
	}
	if((status = dhi2c_read(mAddress, buf, 14)) != DHI2C_OK) {
		dhdebug("mpu6050: failed to read");
		return status;
	}

	buf[14] = 0x6B; // power down
	buf[15] = 1 << 6; // sleep bit
	if((status = dhi2c_write(mAddress, &buf[14], 2, 1)) != DHI2C_OK) {
		dhdebug("mpu6050: failed to power up");
		return status;
	}

	if(acceleromter) {
		acceleromter->X = signedInt16be(buf, 0) * 8.0f * EARTH_GRAVITY_ACCELERATION / 32768.0f;
		acceleromter->Y = signedInt16be(buf, 2) * 8.0f * EARTH_GRAVITY_ACCELERATION / 32768.0f;
		acceleromter->Z = signedInt16be(buf, 4) * 8.0f * EARTH_GRAVITY_ACCELERATION / 32768.0f;
	}

	if(gyroscope) {
		gyroscope->X = signedInt16be(buf, 8) * 1000.0f / 32768.0f;
		gyroscope->Y = signedInt16be(buf, 10) * 1000.0f / 32768.0f;
		gyroscope->Z = signedInt16be(buf, 12) * 1000.0f / 32768.0f;
	}

	if(temparature)
		*temparature = signedInt16be(buf, 6) / 340.0f + 36.53f;
	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR mpu6050_set_address(int address) {
	mAddress = address;
}
