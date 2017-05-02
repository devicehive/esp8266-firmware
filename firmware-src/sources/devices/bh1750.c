/**
 * @file
 * @brief Simple communication with BH1750 illuminance sensor
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/bh1750.h"
#include "DH/i2c.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

#if defined(DH_DEVICE_BH1750)

/** Default sensor i2c address*/
#define BH1750_DEFAULT_ADDRESS 0x46

// module variables
static int mAddress = BH1750_DEFAULT_ADDRESS;


/*
 * bh1750_read() implementation.
 */
int ICACHE_FLASH_ATTR bh1750_read(int sda, int scl, float illuminance[1])
{
	int status;
	if (sda != DH_I2C_NO_PIN && scl != DH_I2C_NO_PIN) {
		if ((status = dh_i2c_init(sda, scl)) != DH_I2C_OK) {
			dhdebug("bh1750: failed to set up pins");
			return status;
		}
	}

	char buf[2];
	buf[0] = 0x21; // One Time High Resolution (0.5 lx)
	if ((status = dh_i2c_write(mAddress, buf, 1, 0)) != DH_I2C_OK) {
		dhdebug("bh1750: failed to measure");
		return status;
	}

	delay_ms(180);
	if ((status = dh_i2c_read(mAddress, buf, 2)) != DH_I2C_OK) {
		dhdebug("bh1750: failed to read");
		return status;
	}

	*illuminance = (float)unsignedInt16be(buf, 0) / 1.2f / 2.0f;
	return DH_I2C_OK;
}


/*
 * bh1750_set_address() implementation.
 */
void ICACHE_FLASH_ATTR bh1750_set_address(int address)
{
	mAddress = address;
}

#endif /* DH_DEVICE_BH1750 */
