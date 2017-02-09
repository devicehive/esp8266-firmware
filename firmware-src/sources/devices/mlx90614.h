/**
 *	\file		mlx90614.h
 *	\brief		Simple communication with MLX90614 contactless IR temperature sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_MLX90614_H_
#define SOURCES_DEVICES_MLX90614_H_

#include "dhi2c.h"

/** Default sensor i2c address*/
#define MLX90614_DEFAULT_ADDRESS 0xB4
/** Do not initialize pin */
#define MLX90614_NO_PIN -1

/**
 *	\brief					Measure temperature one time.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\param[out]	ambient		Pointer for storing ambient temperature result measure in Celsius.
 *	\param[out]	object		Pointer for storing object temperature result measure in Celsius.
 *	\return 				Status value, one of DHI2C_STATUS enum.
 */
DHI2C_STATUS mlx90614_read(int sda, int scl, float *ambient, float *object);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void mlx90614_set_address(int address);

#endif /* SOURCES_DEVICES_MLX90614_H_ */
