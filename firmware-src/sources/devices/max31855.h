/**
 *	\file		max31855.h
 *	\brief		Simple communication with MAX31855 thermocouple temperature sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_MAX31855_H_
#define SOURCES_DEVICES_MAX31855_H_

/** Do not initialize pin */
#define MAX31855_NO_PIN -2

/**
 *	\brief					Measure temperature one time.
 *	\param[in]	cs			Chip select pin. Can be MAX31855_NO_PIN to disable CS.
 *	\param[out]	temperature	Pointer to store measure result in degree Celsius.
 *	\return 				NULL on success, text description on error.
 */
char *max31855_read(int pin, float *temperature);

#endif /* SOURCES_DEVICES_MAX31855_H_ */
