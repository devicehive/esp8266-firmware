/**
 * @file
 * @brief Simple communication with DHT11 humidity sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_DHT_H_
#define _DEVICES_DHT_H_

#include <c_types.h>

/**
 * @brief Measure relative humidity with DHT11 sensor one time.
 * @param[in] pin 1-wire pin for communication. Can be DH_ONEWIRE_NO_PIN.
 * @param[out] humidity Pointer for storing relative humidity result measure in percents.
 * @param[out] temperature Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 * @return NULL on success, text description on error.
 */
const char* dht11_read(int pin, int *humidity, int *temperature);


/**
 * @brief Measure relative humidity with DHT22 sensor one time.
 * @param[in] pin 1-wire pin for communication. Can be DH_ONEWIRE_NO_PIN.
 * @param[out] humidity Pointer for storing relative humidity result measure in percents.
 * @param[out] temperature Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 * @return NULL on success, text description on error.
 */
const char* dht22_read(int pin, float *humidity, float *temperature);


/**
 * @brief Read data from DHT like devices.
 *
 * Checksum will not be checked.
 *
 * @param[out] buf Buffer to read to.
 * @param[in] len Buffer length in bytes.
 * @return Number of read bytes on success, zero on error.
 */
int dht_read(void *buf, size_t len);

#endif /* _DEVICES_DHT_H_ */
