/**
 *	\file		max6675.h
 *	\brief		Simple communication with MAX6675 K-thermocouple temperature sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_MAX6675_H_
#define SOURCES_DEVICES_MAX6675_H_

/** Do not initialize pin */
#define MAX6675_NO_PIN -2

/**
 *	\brief					Measure temperature one time.
 *	\param[in]	cs			Chip select pin. Can be MAX6675_NO_PIN to disable CS.
 *	\param[out]	temperature	Pointer to store measure result in degree Celsius.
 *	\return 				NULL on success, text description on error.
 */
char *max6675_read(int pin, float *temperature);

#endif /* SOURCES_DEVICES_MAX6675_H_ */
