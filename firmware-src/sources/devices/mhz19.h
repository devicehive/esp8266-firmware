/**
 * @file
 * @brief Simple communication with MHZ-19 CO2 sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DEVICES_MHZ19_H_
#define _DEVICES_MHZ19_H_

/**
 * @brief Read CO2 sensor value.
 * @param[out] co2 Pointer to store measure result in ppm(parts-per-million).
 * @return NULL on success, or string with error description.
 */
const char* mhz19_read(int *co2);

#endif /* _DEVICES_MHZ19_H_ */
