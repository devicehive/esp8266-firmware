/**
 * @file
 * @brief Simple communication with PCF8574 GPIO extender.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_PCF8574_H_
#define _DEVICES_PCF8574_H_

#include "user_config.h"
#if defined(DH_DEVICE_PCF8574)

/** @brief Pins which can be used with extender. */
#define PCF8574_SUITABLE_PINS 0b11111111


/**
 * @brief Read data on extender pins.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[out] pins Pointer where pin mask will be stored.
 * @return Status value, one of DH_I2C_Status enum.
 */
int pcf8574_read(int sda, int scl, unsigned int *pins);


/**
 * @brief Write data on extender pins.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[in] pins_to_set Pin mask to set up high level.
 * @param[in] pins_to_clear Pin mask to set up low level.
 * @return Status value, one of DH_I2C_Status enum.
 */
int pcf8574_write(int sda, int scl,
                  unsigned int pins_to_set,
                  unsigned int pins_to_clear);


/**
 * @brief Set all pins.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN.
 * @param[in] pins Pin mask of pins state.
 * @return Status value, one of DH_I2C_Status enum.
 */
int pcf8574_set(int sda, int scl,
                unsigned int pins);


/**
 * @brief Set extender address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void pcf8574_set_address(int address);

#endif /* DH_DEVICE_PCF8574 */
#endif /* _DEVICES_PCF8574_H_ */
