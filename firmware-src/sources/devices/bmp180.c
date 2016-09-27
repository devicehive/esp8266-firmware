/*
 * bmp180.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "bmp180.h"
#include "dhi2c.h"
#include "dhdebug.h"

LOCAL unsigned int ICACHE_FLASH_ATTR unsignedInt16(const char *buf, int pos) {
	return ((int)buf[pos] * 0x100 + (int)buf[pos + 1]);
}

LOCAL int ICACHE_FLASH_ATTR signedInt16(const char *buf, int pos) {
	int r = unsignedInt16(buf, pos);
	if (r <= 0x7FFF)
		return r;
	return r - 0x10000;
}

int ICACHE_FLASH_ATTR bmp180_read(int sda, int scl, float *temperature) {
	char buf[22];
	const int address = 0xEE;
	unsigned int raw_temperature;
	unsigned int raw_pressure;
	if (dhi2c_init(sda, scl) != DHI2C_OK) {
		dhdebug("bmp180: failed to set up pins");
		return BMP180_ERROR;
	}
	buf[0] = 0xAA; // get factory parameters
	if (dhi2c_write(address, buf, 1, 0) != DHI2C_OK) {
		dhdebug("bmp180: failed to write get coeficients command");
		return BMP180_ERROR;
	}
	if (dhi2c_read(address, buf, sizeof(buf)) != DHI2C_OK) {
		dhdebug("bmp180: failed to read coeficients");
		return BMP180_ERROR;
	}

	int oss = 0;
    int ac1 = signedInt16(buf, 0);
    int ac2 = signedInt16(buf, 2);
    int ac3 = signedInt16(buf, 4);
    unsigned int ac4 = unsignedInt16(buf, 6);
    unsigned int ac5 = unsignedInt16(buf, 8);
    unsigned int ac6 = unsignedInt16(buf, 10);
    int b1 = signedInt16(buf, 12);
    int b2 = signedInt16(buf, 14);
    //int mb = signedInt16(buf, 16);
    int mc = signedInt16(buf, 18);
    int md = signedInt16(buf, 20);

    buf[0] = 0xF4; // measure temperature
    buf[1] = 0x2E;
    if (dhi2c_write(address, buf, 2, 1) != DHI2C_OK) {
		dhdebug("bmp180: failed to stare measure temperature");
		return BMP180_ERROR;
	}
    os_delay_us(50000);
    buf[0] = 0xF6; // get result
    if (dhi2c_write(address, buf, 1, 0) != DHI2C_OK) {
		dhdebug("bmp180: failed to write get temperature command");
		return BMP180_ERROR;
	}
	if (dhi2c_read(address, buf, 2) != DHI2C_OK) {
		dhdebug("bmp180: failed to read temperature");
		return BMP180_ERROR;
	}
	raw_temperature = unsignedInt16(buf, 0);

    buf[0] = 0xF4; // measure pressure
    buf[1] = 0x34;
    if (dhi2c_write(address, buf, 2, 1) != DHI2C_OK) {
		dhdebug("bmp180: failed to stare measure pressure");
		return BMP180_ERROR;
	}
    os_delay_us(50000);
    buf[0] = 0xF6; // get result
    if (dhi2c_write(address, buf, 1, 0) != DHI2C_OK) {
		dhdebug("bmp180: failed to write get pressure command");
		return BMP180_ERROR;
	}
	if (dhi2c_read(address, buf, 2) != DHI2C_OK) {
		dhdebug("bmp180: failed to read pressure");
		return BMP180_ERROR;
	}
	raw_pressure = unsignedInt16(buf, 0);

	// calc
	long x1 = (raw_temperature - ac6) * ac5 >> 15;
	long x2 = (mc << 11) / (x1 + md);
	long b5 = x1 + x2;
	long t  = (b5 + 8) >> 4;
	if (temperature) {
		*temperature = (t / 10.0f);
	}

	long b6 = b5 - 4000;
	x1 = (b2 * ((b6 * b6) >> 12)) >> 11;
	x2 = (ac2 * b6) >> 11;
	long x3 = x1 + x2;
	long b3 = (((ac1 * 4 + x3) << oss) + 2) >> 2;
	x1 = (ac3 * b6) >> 13;
	x2 = (b1 * ((b6 * b6) >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	unsigned long b4 = (ac4 * (x3 + 32768)) >> 15;
	unsigned long b7 = ((raw_pressure - b3) * (50000 >> oss));
	long p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;
	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	p = p + ((x1 + x2 + 3791) >> 4);

	return p;
}
