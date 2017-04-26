/**
 *	\file		dht.h
 *	\brief		Simple communication with DHT11 humidity sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_DHT_H_
#define SOURCES_DEVICES_DHT_H_

/** Do not initialize pin */
#define DHT_NO_PIN -1

/**
 *	\brief					Measure relative humidity with DHT11 sensor one time.
 *	\param[in]	pin			1-wire pin for communication.
 *	\param[out]	humidity	Pointer for storing relative humidity result measure in percents.
 *	\param[out]	temperature	Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 *	\return 				NULL on success, text description on error.
 */
char *dht11_read(int pin, int *humidity, int *temperature);

/**
 *	\brief					Measure relative humidity with DHT22 sensor one time.
 *	\param[in]	pin			1-wire pin for communication.
 *	\param[out]	humidity	Pointer for storing relative humidity result measure in percents.
 *	\param[out]	temperature	Pointer for storing temperature result measure in degree Celsius. Can be NULL.
 *	\return 				NULL on success, text description on error.
 */
char *dht22_read(int pin, float *humidity, float *temperature);


/**
 *	\brief				Read data from DHT like devices.
 *	\details			Checksum will not be checked.
 *	\param[out]	buf		Buffer for data.
 *	\param[in]	len		Buffer length in bytes.
 *	\return				Number of read bytes on success, zero on error.
 */
int dhonewire_dht_read(char *buf, unsigned int len);

#endif /* SOURCES_DEVICES_DHT_H_ */
