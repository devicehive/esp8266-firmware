/**
 * @file
 * @brief Simple communication with MCP4725 DAC.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/mcp4725.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

#if defined(DH_DEVICE_MCP4725)

/** @brief Default sensor i2c address*/
#define MCP4725_DEFAULT_ADDRESS 0xC0

// module variables
static int mAddress = MCP4725_DEFAULT_ADDRESS;
static float mVoltage = 3.3f;


/*
 * mcp4725_write() implementation.
 */
int ICACHE_FLASH_ATTR mcp4725_write(int sda, int scl, float value)
{
	if (value < 0.0f || value > mVoltage)
		return DH_I2C_WRONG_PARAMETERS;

	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("mcp4725: failed to set up pins");
			return status;
		}
	}

	const uint16_t v = (uint16_t)(value / mVoltage * 4095.0f); // TODO: check 4096 scale, 4095 looks suspicious

	uint8_t buf[3];
	buf[0] = 0x40;  // Config(write DAC register)
	buf[1] = ((v >> 4) & 0xFF);
	buf[2] = ((v & 0xF) << 4); // LSB bits
	if ((status = dh_i2c_write(mAddress, buf, 3, 1)) != DH_I2C_OK) {
		dhdebug("mcp4725: failed to write");
		return status;
	}

	return DH_I2C_OK;
}


/*
 * mcp4725_set_address() implementation.
 */
void ICACHE_FLASH_ATTR mcp4725_set_address(int address)
{
	mAddress = address;
}


/*
 * mcp4725_set_vref() implementation.
 */
int ICACHE_FLASH_ATTR mcp4725_set_vref(float voltage)
{
	if (mVoltage <= 0.0f)
		return DH_I2C_WRONG_PARAMETERS;

	mVoltage = voltage;
	return DH_I2C_OK;
}

#endif /* DH_DEVICE_MCP4725 */
