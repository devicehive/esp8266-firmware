/**
 *	\file		pcf8574.h
 *	\brief		Simple communication with PCF8574 GPIO extender
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_PCF8574_H_
#define SOURCES_DEVICES_PCF8574_H_

#include "DH/i2c.h"

/** Default sensor i2c address*/
#define PCF8574_DEFAULT_ADDRESS 0x4E
/** Do not initialize pin */
#define PCF8574_NO_PIN -1
/** Pins which can be used with extender */
#define PCF8574_SUITABLE_PINS 0b11111111

/**
 *	\brief				Read data on extender pins.
 *  \param[in]  sda     Pin for I2C's SDA.
 *  \param[in]  scl     Pin for I2C's SCL.
 *	\param[out]	pins	Pointer where pin mask will be stored.
 *	\return 			Status value, one of DH_I2C_Status enum.
 */
DH_I2C_Status pcf8574_read(int sda, int scl, unsigned int *pins);

/**
 *  \brief              		Write data on extender pins.
 *  \param[in]  sda     		Pin for I2C's SDA.
 *  \param[in]  scl     		Pin for I2C's SCL.
 *  \param[in]  pins_to_set		Pin mask to set up high level.
 *  \param[in]  pins_to_clear	Pin mask to set up low level.
 *  \return             		Status value, one of DH_I2C_Status enum.
 */
DH_I2C_Status pcf8574_write(int sda, int scl, unsigned int pins_to_set, unsigned int pins_to_clear);

/**
 *  \brief              Set all pins.
 *  \param[in]  sda     Pin for I2C's SDA.
 *  \param[in]  scl     Pin for I2C's SCL.
 *  \param[in]  pins	Pin mask of pins state.
 *  \return             Status value, one of DH_I2C_Status enum.
 */
DH_I2C_Status pcf8574_set(int sda, int scl, unsigned int pins);

/**
 *	\brief					Set extender address which should be used while reading.
 *	\param[in]	address		Pin for I2C's SDA.
 */
void pcf8574_set_address(int address);

#endif /* SOURCES_DEVICES_PCF8574_H_ */
