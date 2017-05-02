/**
 * @file
 * @brief Simple communication with HD44780 like displays via PCF8574 GPIO extender with I2C bus.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_PCF8574_HD44780_H_
#define _DEVICES_PCF8574_HD44780_H_

#include <c_types.h>

/**
 * @brief Set text on screen.
 *
 * Old text will be erased. '\n'(0x0A) char is supported.
 *
 * @param[in] sda Pin for I2C's SDA. Use DH_I2C_NO_PIN to prevent initialization.
 * @param[in] scl Pin for I2C's SCL. Use DH_I2C_NO_PIN to prevent initialization.
 * @param[in] text Chars to write on display.
 * @param[in] len Number of chars to write. Data more then 80 bytes is ignored.
 * @return Status value, one of DH_I2C_Status enum.
 */
int pcf8574_hd44780_write(int sda, int scl, const char *text, int len);

#endif /* _DEVICES_PCF8574_HD44780_H_ */
