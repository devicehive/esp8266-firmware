/**
 *	\file		bh1750.h
 *	\brief		Simple communication with BH1750 illuminance sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_BH1750_H_
#define SOURCES_DEVICES_BH1750_H_

/** Value for returning on error*/
#define BH1750_ERROR -274
/** Default sensor i2c address*/
#define BH1750_DEFAULT_ADDRESS 0x46
/** Do not initialize pin */
#define BH1750_NO_PIN -1

/**
 *	\brief					Measure illuminance one time.
 *	\param[in]	sda			Pin for I2C's SDA.
 *	\param[in]	scl			Pin for I2C's SCL.
 *	\return 				Illuminance in lux or BH1750_ERROR on error.
 */
float bh1750_read(int sda, int scl);

/**
 *	\brief					Set sensor address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void bh1750_set_address(int address);

#endif /* SOURCES_DEVICES_BH1750_H_ */
