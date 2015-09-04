/**
 *	\file		dhpwm.h
 *	\brief		Software PWM implementation for ESP8266.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 *	\details 	This module uses ESP8266 hardware timer, so any other module with hardware timer requirement will be incompatible.
 */

#ifndef _DHPWM_H_
#define _DHPWM_H_

/** Default frequency in microseconds. */
#define DHPWM_DEFAULT_PERIOD_US 1000
/** PWM depth, high value may cause very high CPU load. */
#define DHPWM_DEPTH 100

/**
 *	\brief					Start PWM for specified pins.
 *	\param[in]	pinsduty	Array with pins duty cycles for each pin.
 *	\param[in]	pinsmask	Bitwise pins mask with pins that should be enabled, this pins have to have correct value in pinsduty array.
 *	\param[in]	periodus	PWM period.
 *	\param[in]	count		Number of tacts for PWM. If zero PWM will not stop automatically.
 *	\return					Non zero value on success or zero on failure.
 */
int dhpwm_set_pwm(unsigned int *pinsduty, unsigned int pinsmask, unsigned int periodus, unsigned int count);

/**
 *	\brief		Get current PWM period.
 *	\return		PWM period in microseconds.
 */
unsigned int dhpwm_get_period_us();

/**
 *	\brief					Stops PWM for specified pins.
 *	\details				PWM timer will stop automatically if no pins left.
 *	\param[in]	pinsmask	Bitwise pins mask.
 */
void dhpwm_disable_pins(unsigned int pinsmask);

#endif /* _DHPWM_H_ */
