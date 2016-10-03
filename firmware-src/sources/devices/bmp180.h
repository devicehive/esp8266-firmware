/**
 *	\file		bmp180.h
 *	\brief		Simple communication with BMP180 pressure sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_BMP180_H_
#define SOURCES_DEVICES_BMP180_H_

#define BMP180_ERROR -274

/**
 *	\brief					Measure pressure one time.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	temperature	Pointer for storing temperature result measure in Celsius. Can be NULL.
 *	\return 				Pressure in Pascals or BMP180_ERROR on error.
 */
int bmp180_read(int sda, int scl, float *temperature);

#endif /* SOURCES_DEVICES_BMP180_H_ */
