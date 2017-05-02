/**
 * @file
 * @brief Simple communication with INA219 power monitor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/ina219.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <ets_forward.h>

#if defined(DH_DEVICE_INA219)

/** @brief Default sensor i2c address*/
#define INA219_DEFAULT_ADDRESS 0x80

// module variables
static int mAddress = INA219_DEFAULT_ADDRESS;
static float mResistance = 0.1f;


/*
 * ina219_read() implementation.
 */
int ICACHE_FLASH_ATTR ina219_read(int sda, int scl, float *voltage, float *current, float *power)
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("ina219: failed to set up pins");
			return status;
		}
	}

	const uint16_t cal = (int)(4096.0f / mResistance);

	uint8_t buf[12];
	buf[0] = 0x05;
	buf[1] = ((cal >> 8) & 0xFF);
	buf[2] = (cal & 0xFF);
	if ((status = dh_i2c_write(mAddress, buf, 3, 1)) != DH_I2C_OK) {
		dhdebug("ina219: failed to write calibration data");
		return status;
	}

	os_delay_us(600);

	int i;
	for (i = 1; i < 5; i++) {
		const uint8_t addr = i;
		if ((status = dh_i2c_write(mAddress, &addr, 1, 0)) != DH_I2C_OK) {
			dhdebug("ina219: failed to write address");
			return status;
		}
		if ((status = dh_i2c_read(mAddress, &buf[i*2], 2)) != DH_I2C_OK) {
			dhdebug("ina219: failed to read data");
			return status;
		}
	}

	// float shunt_voltage = ((float)signedInt16be(buf, 2)) / 100000.0f;
	*voltage = (((float)((unsigned int)buf[4]) * 32 + ((char)buf[5] > 3))) / 8000.0f * 32.0f;
	*current = ((float)signedInt16be((const char*)buf, 8)) / 100000.0f;
	*power = ((float)unsignedInt16be((const char*)buf, 6)) / 100000.0f * 20.0f;

	return DH_I2C_OK;
}


/*
 * ina219_set_address() implementation.
 */
void ICACHE_FLASH_ATTR ina219_set_address(int address)
{
	mAddress = address;
}


/*
 * ina219_set_shunt() implementation.
 */
int ICACHE_FLASH_ATTR ina219_set_shunt(float resistance)
{
	if (mResistance <= 0.0f) // TODO: check 4096.0f/mResistance < 64*1024
		return DH_I2C_WRONG_PARAMETERS;

	mResistance = resistance;
	return DH_I2C_OK;
}

#endif /* DH_DEVICE_INA219 */
