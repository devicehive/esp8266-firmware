/**
 *	\file		dhsender_enums.h
 *	\brief		Declaration of enums for dhsender.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHSENDER_ENUMS_H_
#define _DHSENDER_ENUMS_H_

/** Data type that should be read from arguments and how it will be formatted in response or notification. */
typedef enum {
	RDT_CONST_STRING,	///< Constant pointer to char should be passed. Will be formatted as string.
	RDT_DATA_WITH_LEN,	///< Pointer to data and integer length of data should be passed. Will be formatted as json with current data encoded method.
	RDT_FLOAT,			///< Float should be passed. Will be formatted as json with this value.
	RDT_GPIO,			///< Three 32bit value should be passed(caused, state, tick). Will be formatted as json.
	RDT_SEARCH64,		///< Data with groups of 64bit addresses. Pin number, pointer to data and integer length of data should be passed.
} REQUEST_DATA_TYPE;

/** Request type. */
typedef enum {
	RT_RESPONCE_OK,		///< Response with success status.
	RT_RESPONCE_ERROR,	///< Response with error status.
	RT_NOTIFICATION		///< Notification, see REQUEST_NOTIFICATION_TYPE for available notification types.
} REQUEST_TYPE;

/** Notification type. */
typedef enum {
	RNT_NOTIFICATION_NONE,		///< Dummy, just to pass it for responses.
	RNT_NOTIFICATION_GPIO,		///< Notification will be marked as GPIO.
	RNT_NOTIFICATION_ADC,		///< Notification will be marked as ADC.
	RNT_NOTIFICATION_UART,		///< Notification will be marked as UART.
	RNT_NOTIFICATION_ONEWIRE	///< Notification will be marked as onewire.
} REQUEST_NOTIFICATION_TYPE;

#endif /* _DHSENDER_ENUMS_H_ */
