/**
 *	\file		ds18b20.h
 *	\brief		Simple communication with DS18B20 temperature sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_DS18B20_H_
#define SOURCES_DEVICES_DS18B20_H_

/** Do not initialize pin */
#define DS18B20_NO_PIN -1

/**
 *	\brief					Measure temperature one time.
 *	\param[in]	pin			1-wire pin for communication.
 *	\param[out]	temperature	Pointer to store measure result in degree Celsius.
 *	\return 				NULL on success, text description on error.
 */
char *ds18b20_read(int pin, float *temperature);

#endif /* SOURCES_DEVICES_DS18B20_H_ */
