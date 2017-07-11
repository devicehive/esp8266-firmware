/**
 * @file
 * @brief Simple communication with PCF8591 ADC/DAC.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/pcf8591.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <ets_forward.h>

#if defined(DH_DEVICE_PCF8591)

/** @brief Default sensor i2c address. */
#define PCF8591_DEFAULT_ADDRESS 0x90

// module variables
static int mAddress = PCF8591_DEFAULT_ADDRESS;
static float mVoltage = 3.3f;


/*
 * pcf8591_read() implementation.
 */
int ICACHE_FLASH_ATTR pcf8591_read(int sda, int scl, float values[4])
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("pcf8591: failed to set up pins");
			return status;
		}
	}

	uint8_t buf[4];
	buf[0] = 0x45;  // Config
	if ((status = dh_i2c_write(mAddress, buf, 1, 1)) != DH_I2C_OK) {
		dhdebug("pcf8591: failed to write");
		return status;
	}

	// dummy read to make sure all values was updated
	if ((status = dh_i2c_read(mAddress, buf, 4)) != DH_I2C_OK) {
		dhdebug("pcf8591: failed to read");
		return status;
	}
	os_delay_us(250);
	if ((status = dh_i2c_read(mAddress, buf, 4)) != DH_I2C_OK) {
		dhdebug("pcf8591: failed to read");
		return status;
	}

	int i;
	for (i = 0; i < sizeof(buf); i++) {
		values[i] = mVoltage * ((float)buf[i]) / 255.0f;
	}

	return DH_I2C_OK;
}


/*
 * pcf8591_write() implementation.
 */
int ICACHE_FLASH_ATTR pcf8591_write(int sda, int scl, float value)
{
	if (value < 0.0f || value > mVoltage)
		return DH_I2C_WRONG_PARAMETERS;

	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("pcf8591: failed to set up pins");
			return status;
		}
	}

	const uint8_t v = (uint8_t)(value / mVoltage * 255.0f);

	uint8_t buf[2];
	buf[0] = 0x44;  // Config
	buf[1] = v;
	if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
		dhdebug("pcf8591: failed to write");
		return status;
	}

	return DH_I2C_OK;
}


/*
 * pcf8591_set_address() implementation.
 */
void ICACHE_FLASH_ATTR pcf8591_set_address(int address)
{
	mAddress = address;
}


/*
 * pcf8591_set_vref() implementation.
 */
int ICACHE_FLASH_ATTR pcf8591_set_vref(float voltage)
{
	if (mVoltage <= 0.0f)
		return DH_I2C_WRONG_PARAMETERS;

	mVoltage = voltage;
	return DH_I2C_OK;
}

#endif /* DH_DEVICE_PCF8591 */
