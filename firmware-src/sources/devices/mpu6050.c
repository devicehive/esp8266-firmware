/**
 * @file
 * @brief Simple communication with MPU6050 accelerometer and gyroscope sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/mpu6050.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

/** @brief Default sensor i2c address*/
#define MPU6050_DEFAULT_ADDRESS 0xD0

// module variables
static int mAddress = MPU6050_DEFAULT_ADDRESS;
#define EARTH_GRAVITY_ACCELERATION 9.80665f


/*
 * mpu6050_read() implementation.
 */
int ICACHE_FLASH_ATTR mpu6050_read(int sda, int scl, MPU6050_XYZ *acceleromter, MPU6050_XYZ *gyroscope, float *temparature)
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("mpu6050: failed to set up pins");
			return status;
		}
	}

	uint8_t buf[16];
	buf[0] = 0x6B; // power up
	buf[1] = 0; // no sleep bit
	if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
		dhdebug("mpu6050: failed to power up");
		return status;
	}

	buf[0] = 0x1C; // accelerometer configuration
	buf[1] = BIT(4); // AFS_SEL = 2, range +-8 g
	if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
		dhdebug("mpu6050: failed to configure accelerometer");
		return status;
	}

	buf[0] = 0x1B; // gyroscope configuration
	buf[1] = BIT(4); // FS_SEL = 2, range +-1000 dps
	if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
		dhdebug("mpu6050: failed to configure gyroscope");
		return status;
	}

	delay_ms(50);

	buf[0] = 0x3B; // get data
	if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("mpu6050: failed to set read register");
		return status;
	}
	if((status = dh_i2c_read(mAddress, buf, 14)) != DH_I2C_OK) {
		dhdebug("mpu6050: failed to read");
		return status;
	}

	buf[14] = 0x6B; // power down
	buf[15] = 1 << 6; // sleep bit
	if ((status = dh_i2c_write(mAddress, &buf[14], 2, 1)) != DH_I2C_OK) {
		dhdebug("mpu6050: failed to power up");
		return status;
	}

	if (acceleromter) {
		acceleromter->X = signedInt16be((const char*)buf, 0) * 8.0f * EARTH_GRAVITY_ACCELERATION / 32768.0f;
		acceleromter->Y = signedInt16be((const char*)buf, 2) * 8.0f * EARTH_GRAVITY_ACCELERATION / 32768.0f;
		acceleromter->Z = signedInt16be((const char*)buf, 4) * 8.0f * EARTH_GRAVITY_ACCELERATION / 32768.0f;
	}

	if (gyroscope) {
		gyroscope->X = signedInt16be((const char*)buf, 8) * 1000.0f / 32768.0f;
		gyroscope->Y = signedInt16be((const char*)buf, 10) * 1000.0f / 32768.0f;
		gyroscope->Z = signedInt16be((const char*)buf, 12) * 1000.0f / 32768.0f;
	}

	if (temparature)
		*temparature = signedInt16be((const char*)buf, 6) / 340.0f + 36.53f;

	return DH_I2C_OK;
}


/*
 * mpu6050_set_address() implementation.
 */
void ICACHE_FLASH_ATTR mpu6050_set_address(int address)
{
	mAddress = address;
}
