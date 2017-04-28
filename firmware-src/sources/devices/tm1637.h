/**
 * @file
 * @brief Simple communication with TM1637 LED 8 segment driver.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_TM1637_H_
#define _DEVICES_TM1637_H_

#include <c_types.h>

/**
 * @brief Set text on screen.
 *
 * Old text will be erased.
 *
 * @param[in] sda Pin for I2C's SDA. Use DH_I2C_NO_PIN to prevent initialization.
 * @param[in] scl Pin for I2C's SCL. Use DH_I2C_NO_PIN to prevent initialization.
 * @param[in] text Chars to write on display. Only digits and colon are allowed.
 * @param[in] len Number of chars to write.
 * @return Status value, one of DH_I2C_Status enum.
 */
int tm1636_write(int sda, int scl, const char *text, size_t len);

#endif /* _DEVICES_TM1637_H_ */
