/**
 *	\file		bmp280.h
 *	\brief		Simple communication with BMP280 pressure sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_BMP280_H_
#define SOURCES_DEVICES_BMP280_H_

#include "DH/i2c.h"

/** Default sensor i2c address*/
#define BMP280_DEFAULT_ADDRESS 0xEC
/** Do not initialize pin */
#define BMP280_NO_PIN -1

/**
 *	\brief					Measure pressure one time.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	pressure	Pointer for storing pressure result measure in Pascals.
 *	\param[out]	temperature	Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 *	\return 				Status value, one of DH_I2C_Status enum.
 */
DH_I2C_Status bmp280_read(int sda, int scl, float *pressure, float *temperature);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void bmp280_set_address(int address);

#endif /* SOURCES_DEVICES_BMP280_H_ */
