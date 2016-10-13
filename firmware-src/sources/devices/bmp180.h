/**
 *	\file		bmp180.h
 *	\brief		Simple communication with BMP180 pressure sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_BMP180_H_
#define SOURCES_DEVICES_BMP180_H_

/** Value for returning on error*/
#define BMP180_ERROR -274
/** Default sensor i2c address*/
#define BMP180_DEFAULT_ADDRESS 0xEE
/** Do not initialize pin */
#define BMP180_NO_PIN -1

/**
 *	\brief					Measure pressure one time.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	temperature	Pointer for storing temperature result measure in Celsius. Can be NULL.
 *	\return 				Pressure in Pascals or BMP180_ERROR on error.
 */
int bmp180_read(int sda, int scl, float *temperature);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void bmp180_set_address(int address);

#endif /* SOURCES_DEVICES_BMP180_H_ */
