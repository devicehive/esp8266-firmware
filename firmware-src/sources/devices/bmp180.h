/**
 *	\file		bmp180.h
 *	\brief		Simple communication with BMP180 pressure sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_BMP180_H_
#define SOURCES_DEVICES_BMP180_H_

#include "dhi2c.h"

/** Default sensor i2c address*/
#define BMP180_DEFAULT_ADDRESS 0xEE
/** Do not initialize pin */
#define BMP180_NO_PIN -1

/**
 *	\brief					Measure pressure one time.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	pressure	Pointer for storing pressure result measure in Pascals.
 *	\param[out]	temperature	Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 *	\return 				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS bmp180_read(int sda, int scl, int *pressure, float *temperature);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void bmp180_set_address(int address);

#endif /* SOURCES_DEVICES_BMP180_H_ */
