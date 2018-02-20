/**
 * @file
 * @brief Simple communication with BMP280 and BME280 sensors.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/bmx280.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

#if defined(DH_DEVICE_BMX280)

/** @brief Default sensor i2c address*/
#define BMX280_DEFAULT_ADDRESS 0xEC

// module variables
static int mAddress = BMX280_DEFAULT_ADDRESS;


/*
 * bmx280_read() implementation.
 */
int ICACHE_FLASH_ATTR bmx280_read(int sda, int scl, float *pressure, float *temperature, float *humidity)
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("bmx280: failed to set up pins");
			return status;
		}
	}

	char buf[26];
	buf[0] = 0x88; // get factory parameters
	if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("bmx280: failed to write get coefficients command");
		return status;
	}
	if ((status = dh_i2c_read(mAddress, buf, sizeof(buf))) != DH_I2C_OK) {
		dhdebug("bmx280: failed to read coefficients");
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
	double H1 = (double)((unsigned char)buf[25]);

	double H2 = 0.0;
	double H3 = 0.0;
	double H4 = 0.0;
	double H5 = 0.0;
	double H6 = 0.0;

	if (humidity) {
		buf[0] = 0xE1; // get factory parameters for humidity
		if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
			dhdebug("bmx280: failed to write get humidity coefficients command");
			return status;
		}
		if ((status = dh_i2c_read(mAddress, buf, 7)) != DH_I2C_OK) {
			dhdebug("bmx280: failed to read humidity coefficients");
			return status;
		}
		H2 = (double)signedInt16le(buf, 0);
		H3 = (double)((unsigned char)buf[2]);
		signed short v = buf[3];
		v = v << 4;
		v |= buf[4] & 0b00001111;
		H4 = (double)v;
		v = buf[5];
		v = v << 4;
		v |= buf[4] >> 4;
		H5 = (double)v;
		H6 = (double)((signed char)buf[6]);
	}


	// configure
	buf[0] = 0xF5; // config
	buf[1] = 0x0C; // filter coefficient 8, spi off, duration 0
	if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
		dhdebug("bmx280: failed to configure");
		return status;
	}
	buf[0] = 0xF4; // control
	buf[1] = 0xB7; // both oversampling to x16, normal mode
	if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
		dhdebug("bmx280: failed to control");
		return status;
	}

	do { // wait until data is ready
		buf[0] = 0xF3; // status
		if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
			dhdebug("bmx280: failed to get status");
			return status;
		}
		if ((status = dh_i2c_read(mAddress, buf, 1)) != DH_I2C_OK) {
			dhdebug("bmx280: failed to read status");
			return status;
		}
	} while (buf[0] & BIT(3));

	buf[0] = 0xF7; // read press and temp result
	if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("bmx280: failed to start reading");
		return status;
	}
	if ((status = dh_i2c_read(mAddress, buf, 6)) != DH_I2C_OK) {
		dhdebug("bmx280: failed to read");
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
	double t_fine = v1 + v2;
	if (temperature) {
		*temperature = (float)t_fine / 5120.0f;
	}

	v1 = (t_fine) / 2.0 - 64000.0;
	v2 = v1 * v1 * P6 / 32768.0;
	v2 = v2 + v1 * P5 * 2.0;
	v2 = v2 / 4.0 + P4 * 65536.0;
	v1 = (P3 * v1 * v1 / 524288.0 + P2 * v1) / 524288.0;
	v1 = (1.0 + v1 / 32768.0) * P1;
	if (v1 == 0.0) { // TODO: fix this condition
		return DH_I2C_DEVICE_ERROR;
	}
	double p = 1048576.0 - raw_pressure;
	p = (p - v2 / 4096.0) * 6250.0 / v1;
	v1 = P9 * p * p / 2147483648.0;
	v2 = p * P8 / 32768.0;
	p = p + (v1 + v2 + P7) / 16.0;
	*pressure = (float)p;

	if (humidity) {
		buf[0] = 0xFD; // read humidity
		if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
			dhdebug("bmx280: failed to start reading humidity");
			return status;
		}
		if ((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
			dhdebug("bmx280: failed to read humidity");
			return status;
		}
		double h = t_fine - 76800.0;
		h =  (signedInt16le(buf, 0) - (H4 * 64.0 + H5 / 16384.0 * h)) * \
				(H2 / 65536.0 * (1.0 + H6 / 67108864.0 * h * (1.0 + H3 / 67108864.0 * h)));
		h = h * (1.0 - H1 * h / 524288.0);
		if (h > 100.0) {
			h = 100.0;
		} else if (h < 0.0) {
			h = 0.0;
		}
		*humidity = (float)h;
	}

	buf[0] = 0xF4; // control
	buf[1] = 0x0;  // sleep mode
	if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
		dhdebug("bmx280: failed to shutdown");
		return status;
	}

	return DH_I2C_OK;
}


/*
 * bmx280_set_address() implementation.
 */
void ICACHE_FLASH_ATTR bmx280_set_address(int address)
{
	mAddress = address;
}

#endif /* DH_DEVICE_BMX280 */
