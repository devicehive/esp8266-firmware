/*
 * bmp280.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "bmp280.h"
#include "dhi2c.h"
#include "dhdebug.h"
#include "dhutils.h"

static int mAddress = BMP280_DEFAULT_ADDRESS;

DHI2C_STATUS ICACHE_FLASH_ATTR bmp280_read(int sda, int scl, float *pressure, float *temperature) {
	char buf[24];
	DHI2C_STATUS status;
	if(sda != BMP280_NO_PIN && scl != BMP280_NO_PIN) {
		if((status = dhi2c_init(sda, scl)) != DHI2C_OK) {
			dhdebug("bmp280: failed to set up pins");
			return status;
		}
	}
	buf[0] = 0x88; // get factory parameters
	if((status = dhi2c_write(mAddress, buf, 1, 0)) != DHI2C_OK) {
		dhdebug("bmp280: failed to write get coefficients command");
		return status;
	}
	if((status = dhi2c_read(mAddress, buf, sizeof(buf))) != DHI2C_OK) {
		dhdebug("bmp280: failed to read coefficients");
		return status;
	}
	double T1 = (double)unsignedInt16le(buf, 0);
	double T2 = (double)signedInt16le(buf, 2);
	double T3 = (double)signedInt16le(buf, 4);
	double P1 = (double)unsignedInt16le(buf, 6);
	double P2 = (double)signedInt16le(buf, 8);
	double P3 = (double)signedInt16le(buf, 10);
	double P4 = (double)signedInt16le(buf, 12);
	double P5 = (double)signedInt16le(buf, 14);
	double P6 = (double)signedInt16le(buf, 16);
	double P7 = (double)signedInt16le(buf, 18);
	double P8 = (double)signedInt16le(buf, 20);
	double P9 = (double)signedInt16le(buf, 22);

	// configure
	buf[0] = 0xF5; // config
	buf[1] = 0x0C; // filter coefficient 8, spi off, duration 0
	if((status = dhi2c_write(mAddress, buf, 2, 1)) != DHI2C_OK) {
		dhdebug("bmp280: failed to configure");
		return status;
	}
	buf[0] = 0xF4; // control
	buf[1] = 0xB7; // both oversampling to x16, normal mode
	if((status = dhi2c_write(mAddress, buf, 2, 1)) != DHI2C_OK) {
		dhdebug("bmp280: failed to configure");
		return status;
	}

	do { // wait until data is ready
		buf[0] = 0xF3; // status
		if((status = dhi2c_write(mAddress, buf, 1, 0)) != DHI2C_OK) {
			dhdebug("bmp280: failed to get status");
			return status;
		}
		if((status = dhi2c_read(mAddress, buf, 1)) != DHI2C_OK) {
			dhdebug("bmp280: failed to read status");
			return status;
		}
	} while(buf[0] & (1<<3));

	buf[0] = 0xF7; // read press and temp result
	if((status = dhi2c_write(mAddress, buf, 1, 0)) != DHI2C_OK) {
		dhdebug("bmp280: failed to start reading");
		return status;
	}
	if((status = dhi2c_read(mAddress, buf, 6)) != DHI2C_OK) {
		dhdebug("bmp280: failed to read");
		return status;
	}
	buf[2] &= 0xF0;
	buf[5] &= 0xF0;
	double raw_pressure = (double)((((unsigned int)buf[0]) << 12) +
			(((unsigned int)buf[1]) << 4) + (buf[2] >> 4));
	double raw_temperature = (double)((((unsigned int)buf[3]) << 12) +
			(((unsigned int)buf[4]) << 4) + (buf[5] >> 4));
	double v1 = (raw_temperature / 16384.0 - T1 / 1024.0) * T2;
	double v2 = raw_temperature / 131072.0 - T1 / 8192.0;
	v2 = v2 * v2 * (double)T3;
	long int t_fine = v1 + v2;
	if(temperature) {
		*temperature = (float)t_fine / 5120.0f;
	}

	v1 = ((double)t_fine) / 2.0 - 64000.0;
	v2 = v1 * v1 * P6 / 32768.0;
	v2 = v2 + v1 * P5 * 2.0;
	v2 = v2 / 4.0 + P4 * 65536.0;
	v1 = (P3 * v1 * v1 / 524288.0 + P2 * v1) / 524288.0;
	v1 = (1.0 + v1 / 32768.0) * P1;
	if(v1 == 0.0) {
		return DHI2C_DEVICE_ERROR;
	}
	double p = 1048576.0 - raw_pressure;
	p = (p - v2 / 4096.0) * 6250.0 / v1;
	v1 = P9 * p * p / 2147483648.0;
	v2 = p * P8 / 32768.0;
	p = p + (v1 + v2 + P7) / 16.0;
	*pressure = (float)p;

	buf[0] = 0xF4; // control
	buf[1] = 0x0;  // sleep mode
	if((status = dhi2c_write(mAddress, buf, 2, 1)) != DHI2C_OK) {
		dhdebug("bmp280: failed to shutdown");
		return status;
	}
	return DHI2C_OK;
}

void ICACHE_FLASH_ATTR bmp280_set_address(int address) {
	mAddress = address;
}
