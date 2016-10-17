/**
 *	\file		mhz19.h
 *	\brief		Simple communication with MHZ-19 CO2 sensor
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_MHZ19_H_
#define SOURCES_DEVICES_MHZ19_H_

/**
 *	\brief				Read CO2 sensor value.
 *	\param[out]	co2		Pointer to store measure result in ppm(parts-per-million).
 *	\return 			NULL on success, or string with error description.
 */
char *mhz19_read(int *co2);

#endif /* SOURCES_DEVICES_MHZ19_H_ */
