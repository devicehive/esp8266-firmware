/**
 *	\file		dhsender.h
 *	\brief		Module for sending remote command results and notifications
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 *	\details 	Responses and notifications will not be sent immediately, module has queue that use dynamic memory.
 *				Having this queue module can collect lots of responses or notifications that will be sent when it is
 *				possible. But if no dynamic memory enough data will be lost, notification about it will print in debug.
 */

#ifndef _DHSENDER_H_
#define _DHSENDER_H_

#include <ip_addr.h>

#include "dhsender_data.h"

/**
 *	\brief				Notify that current data was failed to send.
 */
void dhsender_current_fail();

/**
 *	\brief				Notify that current data was sent.
 */
void dhsender_current_success();

/**
 *	\brief				Get next struct SENDER_JSON_DATA which should be sent.
 *	\return				Pointer to SENDER_JSON_DATA or NULL if there is no data to send.
 */
SENDER_JSON_DATA *dhsender_next();

/**
 *	\brief					Send command response.
 *	\param[in]	cid			Command id that response should be sent.
 *	\param[in]	status		Command status.
 *	\param[in]	data_type	Data type for response.
 *	\param[in]	...			Data according to the type.
 */
void dhsender_response(CommandResultArgument cid, RESPONCE_STATUS status, REQUEST_DATA_TYPE data_type, ...);

/**
 *	\brief					Send notification.
 *	\param[in]	type		Type of module that sends notification.
 *	\param[in]	data_type	Data type for notification.
 *	\param[in]	...			Data according to the type.
 */
void dhsender_notification(REQUEST_NOTIFICATION_TYPE type, REQUEST_DATA_TYPE data_type, ...);

#endif /* _DHSENDER_H_ */
