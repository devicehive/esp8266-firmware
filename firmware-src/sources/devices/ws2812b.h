/**
 *	\file		ws2812b.h
 *	\brief		Simple communication with WE2812B led controller
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef SOURCES_DEVICES_WS2812B_H_
#define SOURCES_DEVICES_WS2812B_H_

#define WS2812B_NO_PIN -1

/**
 *	\brief					Measure temperature one time.
 *	\param[in]	pin			1-wire pin for communication.
 *	\param[in]	buf			Pointer to buffer with data.
 *	\param[in]	len			Buffer length.
 *	\return 				NULL on success, text description on error.
 */
char *ws2812b_write(int pin, const char *buf, unsigned int len);

#endif /* SOURCES_DEVICES_WS2812B_H_ */
