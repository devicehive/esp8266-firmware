/**
 * @file
 * @brief Simple communication with HMC5883L compass sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/hmc5883l.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

#if defined(DH_DEVICE_HMC5883L)

/** @brief Default sensor i2c address*/
#define HMC5883L_DEFAULT_ADDRESS 0x3C

#define HMC5883l_OVERFLOWED_RAW -4096

// module variables
static int mAddress = HMC5883L_DEFAULT_ADDRESS;


/*
 * hmc5883l_read() implementation.
 */
int ICACHE_FLASH_ATTR hmc5883l_read(int sda, int scl, HMC5883L_XYZ *compass)
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("hmc5883l: failed to set up pins");
			return status;
		}
	}

	uint8_t buf[7];
	buf[0] = 0x02; // mode register
	buf[1] = BIT(0); // single run mode
	if ((status = dh_i2c_write(mAddress, buf, 2, 1)) != DH_I2C_OK) {
		dhdebug("hmc5883l: failed to power up");
		return status;
	}

	delay_ms(80);

	buf[0] = 0x03; // get data
	if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("hmc5883l: failed to set read register");
		return status;
	}
	if ((status = dh_i2c_read(mAddress, buf, 7)) != DH_I2C_OK) {
		dhdebug("hmc5883l: failed to read");
		return status;
	}

	const int x = signedInt16be((const char*)buf, 0);
	const int y = signedInt16be((const char*)buf, 4);
	const int z = signedInt16be((const char*)buf, 2);
	// note: 2048 is a range of possible values, 1.3f is a default sensor field range
	compass->X = (x == HMC5883l_OVERFLOWED_RAW) ?  HMC5883l_OVERFLOWED : (x * 1.3f / 2048.0f);
	compass->Y = (y == HMC5883l_OVERFLOWED_RAW) ?  HMC5883l_OVERFLOWED : (y * 1.3f / 2048.0f);
	compass->Z = (z == HMC5883l_OVERFLOWED_RAW) ?  HMC5883l_OVERFLOWED : (z * 1.3f / 2048.0f);

	return DH_I2C_OK;
}


/*
 * hmc5883l_set_address() implementation.
 */
void ICACHE_FLASH_ATTR hmc5883l_set_address(int address)
{
	mAddress = address;
}

#endif /* DH_DEVICE_HMC5883L */
