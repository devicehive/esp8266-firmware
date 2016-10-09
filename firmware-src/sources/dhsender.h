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
 *	\brief				Initializes sender for using remote DeviceHive server.
 *	\param[in]	ip		Remote server IP.
 *	\param[in]	port	Remote server port.
 */
void dhsender_init(ip_addr_t *ip, int port);

/**
 *	\brief				Start sending data from queue.
 *	\details			It does nothing if data sending is already in progress or there is no data for sending
 */
void dhsender_start();

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

/**
 *	\brief				Stops repeat attempts on error.
 */
void dhsender_stop_repeat();

#endif /* _DHSENDER_H_ */
