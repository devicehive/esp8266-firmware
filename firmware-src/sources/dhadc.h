/**
 *	\file		dhadc.h
 *	\brief		ADC hardware access layer for DeviceHive firmware.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHADC_H_
#define _DHADC_H_

/** ADC suitable channels. */
#define DHADC_SUITABLE_PINS 0b01 // ADC0

/**
 *	\brief				Get ADC value.
 *	\return 			Voltage in volts.
 */
float dhadc_get_value();

/**
 *	\brief				Start loop measurement with some interval.
 *	\param[in]	period	Period in milliseconds.
 */
void dhadc_loop(unsigned int period);

/**
 *	\brief				Callback for loop measurement.
 *	\param[in]	value	Voltage in volts.
 */
extern void dhadc_loop_value(float value);

#endif /* _DHADC_H_ */
