/**
 * @file max31855.h
 * @brief Simple communication with MAX31855 thermocouple temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_MAX31855_H_
#define _DEVICES_MAX31855_H_

#include "user_config.h"
#if defined(DH_DEVICE_MAX31855)

/**
 * @brief Measure temperature one time.
 * @param[in] cs Chip select pin. Can be DH_SPI_NO_PIN to disable CS.
 * @param[out] temperature Pointer to store measure result in degree Celsius.
 * @return NULL on success, text description on error.
 */
const char* max31855_read(int pin, float *temperature);

#endif /* DH_DEVICE_MAX31855 */
#endif /* _DEVICES_MAX31855_H_ */
