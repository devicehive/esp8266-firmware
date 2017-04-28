/**
 * @file
 * @brief Simple communication with MAX6675 K-thermocouple temperature sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_MAX6675_H_
#define _DEVICES_MAX6675_H_


/**
 * @brief Measure temperature one time.
 * @param[in] cs Chip select pin. Can be DH_SPI_NO_PIN to disable CS.
 * @param[out] temperature Pointer to store measure result in degree Celsius.
 * @return NULL on success, text description on error.
 */
const char* max6675_read(int pin, float *temperature);

#endif /* SOURCES_DEVICES_MAX6675_H_ */
