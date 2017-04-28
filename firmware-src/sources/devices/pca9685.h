/**
 * @file
 * @brief Simple communication with PCA9685 PWM LED controller
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_PCA9685_H_
#define _DEVICES_PCA9685_H_

#include "DH/gpio.h"

/** @brief Do not change frequency */
#define PCA9685_NO_PERIOD 0

/** Pins which are allowed for this chip */
#define PCA9685_SUITABLE_PINS 0xFFFF


/**
 * @brief Measure pressure one time.
 * @param[in] sda Pin for I2C's SDA. Can be DH_I2C_NO_PIN to use current.
 * @param[in] scl Pin for I2C's SCL. Can be DH_I2C_NO_PIN to use current.
 * @param[in] pins_duty Array with pins duty cycles for each pin.
 * @param[in] pins Bitwise pins mask with pins that should be enabled,
 *                 this pins have to have correct value in pins_duty array.
 * @param[in] period_us PWM period. Can be PCA9685_NO_PERIOD to use current.
 * @return Status value, one of DH_I2C_Status enum.
 */
int pca9685_control(int sda, int scl, const float pins_duty[DH_GPIO_PIN_COUNT],
                    DHGpioPinMask pins, unsigned int period_us);


/**
 * @brief Set sensor address which should be used while reading.
 * @param[in] address I2C end device address.
 */
void pca9685_set_address(int address);

#endif /* _DEVICES_PCA9685_H_ */
