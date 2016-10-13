/**
 *	\file		dht.h
 *	\brief		Simple communication with DHT11 humidity sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_DHT_H_
#define SOURCES_DEVICES_DHT_H_

/** Value for returning on error*/
#define DHT_ERROR -274
/** Do not initialize pin */
#define DHT_NO_PIN -1

/**
 *	\brief					Measure humidity with DHT11 sensor one time.
 *	\param[in]	pin			1-wire pin for communication.
 *	\param[out]	temperature	Pointer for storing temperature result measure in Celsius. Can be NULL.
 *	\return 				Humidity in percents or DHT_ERROR on error.
 */
int dht11_read(int pin, int *temperature);

/**
 *	\brief					Measure humidity with DHT22 sensor one time.
 *	\param[in]	pin			1-wire pin for communication.
 *	\param[out]	temperature	Pointer for storing temperature result measure in Celsius. Can be NULL.
 *	\return 				Humidity in percents or DHT_ERROR on error.
 */
float dht22_read(int pin, float *temperature);

#endif /* SOURCES_DEVICES_DHT_H_ */
