/**
 *	\file		si7021.h
 *	\brief		Simple communication with SI7021 relative humidity and temperature sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_SI7021_H_
#define SOURCES_DEVICES_SI7021_H_

#include "dhi2c.h"

/** Default sensor i2c address*/
#define SI7021_DEFAULT_ADDRESS 0x80
/** Do not initialize pin */
#define SI7021_NO_PIN -1

/**
 *	\brief					Measure temperature and relative humidity one time.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	humidity	Pointer for storing relative humidity result measure in percents.
 *	\param[out]	temperature	Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 *	\return 				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS si7021_read(int sda, int scl, float *humidity, float *temperature);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void si7021_set_address(int address);

#endif /* SOURCES_DEVICES_SI7021_H_ */
