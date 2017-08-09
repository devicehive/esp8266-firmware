/**
 * @file
 * @brief Simple communication with DS18B20 temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_DS18B20_H_
#define _DEVICES_DS18B20_H_

#include "user_config.h"
#if defined(DH_DEVICE_DS18B20)

/**
 * @brief Measure temperature one time.
 * @param[in] pin 1-wire pin for communication. Can be DH_ONEWIRE_NO_PIN.
 * @param[out] temperature Pointer to store measure result in degree Celsius.
 * @return NULL on success, text description on error.
 */
const char* ds18b20_read(int pin, float *temperature);

#endif /* DH_DEVICE_DS18B20 */
#endif /* _DEVICES_DS18B20_H_ */
