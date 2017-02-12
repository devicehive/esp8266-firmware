/**
 *	\file		tm1636.h
 *	\brief		Simple communication with tm1636 LED 8 segment driver.
 *	\author		Nikolay Khabarov
 *	\date		2017
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_TM1636_H_
#define SOURCES_DEVICES_TM1636_H_

#include "dhi2c.h"

/** Do not initialize pin */
#define TM1636_NO_PIN -1

/**
 *	\brief				Set text on screen.
 *	\details			Old text will be erased.
 *	\param[in]	sda		Pin for I2C's SDA. Use TM1636_NO_PIN to prevent initialization.
 *	\param[in]	scl		Pin for I2C's SCL. Use TM1636_NO_PIN to prevent initialization.
 *	\param[in]	text	Chars to write on display. Only digits and colon are allowed.
  *	\param[in]	len		Number of chars to write.
 *	\return 			NULL on success, text description on error.
 */
DHI2C_STATUS tm1636_write(int sda, int scl, const char *text, unsigned int len);

#endif /* SOURCES_DEVICES_TM1636_H_ */
