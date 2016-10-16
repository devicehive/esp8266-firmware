/**
 *	\file		pcf8574_hd44780.h
 *	\brief		Simple communication with HD44780 like displays via PCF8574 GPIO extender with I2C bus.
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_PCF8574_HD44780_H_
#define SOURCES_DEVICES_PCF8574_HD44780_H_

#include "dhi2c.h"

/**
 *	\brief				Set text on screen.
 *	\details			Old text will be erased. '\n'(0x0A) char is supported.
 *	\param[in]	sda		Pin for I2C's SDA. Use HMC5883L_NO_PIN to prevent initialization.
 *	\param[in]	scl		Pin for I2C's SCL. Use HMC5883L_NO_PIN to prevent initialization.
 *	\param[in]	text	Chars to write on display.
  *	\param[in]	len		Number of chars to write. Data more then 80 bytes is ignored.
 *	\return 			NULL on success, text description on error.
 */
DHI2C_STATUS pcf8574_hd44780_write(int sda, int scl, const char *text, unsigned int len);

#endif /* SOURCES_DEVICES_PCF8574_HD44780_H_ */
