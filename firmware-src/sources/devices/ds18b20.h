/**
 *	\file		ds18b20.h
 *	\brief		Simple communication with DS18B20 temperature sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_DS18B20_H_
#define SOURCES_DEVICES_DS18B20_H_

/** Value for returning on error*/
#define DS18B20_ERROR -274
/** Do not initialize pin */
#define DS18B20_NO_PIN -1

/**
 *	\brief				Measure temperature one time.
 *	\param[in]	pin		1-wire pin for communication.
 *	\return 			Temperature in Celcius or DS18B20_ERROR on error.
 */
float ds18b20_read(int pin);

#endif /* SOURCES_DEVICES_DS18B20_H_ */
