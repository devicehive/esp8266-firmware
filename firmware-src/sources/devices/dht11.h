/**
 *	\file		dht11.h
 *	\brief		Simple communication with DHT11 humidity sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_DHT11_H_
#define SOURCES_DEVICES_DHT11_H_

#define DHT11_ERROR -274

/**
 *	\brief					Measure humidity one time.
 *	\param[in]	pin			1-wire pin for communication.
 *	\param[out]	temperature	Pointer for storing temperature result measure in Celsius. Can be NULL.
 *	\return 				Humidity in percents or DHT11_ERROR on error.
 */
int dht11_read(int pin, int *temperature);

#endif /* SOURCES_DEVICES_DHT11_H_ */
