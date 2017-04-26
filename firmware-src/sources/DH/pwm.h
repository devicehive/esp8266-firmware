/**
 * @file
 * @brief Software PWM implementation for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 *
 * This module uses ESP8266 hardware timer, so any other module
 * with hardware timer requirement will be incompatible.
 */
#ifndef _DH_PWM_H_
#define _DH_PWM_H_

#include "DH/gpio.h"

/**
 * @brief Default PWM frequency in microseconds.
 */
#define DH_PWM_DEFAULT_PERIOD_US 1000

/**
 * @brief PWM depth, high value may cause very high CPU load.
 */
#define DH_PWM_DEPTH 100


/**
 * @brief Start PWM for specified pins.
 * @param[in] duty Array with pins duty cycles for each pin.
 * @param[in] pins Bitwise pin mask with pins that should be enabled,
 *                 these pins must have correct values in `duty` array.
 * @param[in] period_us PWM period, microseconds.
 * @param[in] count Number of tacts for PWM. If zero PWM will not stop automatically.
 * @return Zero on success.
 */
int dh_pwm_start(uint32_t duty[DH_GPIO_PIN_COUNT],
                 DHGpioPinMask pins,
                 unsigned int period_us,
                 unsigned int count);


/**
 * @brief Get current PWM period.
 * @return PWM period in microseconds.
 */
unsigned int dh_pwm_get_period_us(void);


/**
 * @brief Stops PWM for specified pins.
 *
 * PWM timer will stop automatically if no pins left.
 *
 * @param[in] pins Bitwise pins mask.
 */
void dh_pwm_disable(DHGpioPinMask pins);

#endif /* _DH_PWM_H_ */
