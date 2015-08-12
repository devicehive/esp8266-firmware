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

#ifndef DHSENDER_H_
#define DHSENDER_H_

#include <ip_addr.h>

/** Status string for success command */
static const char STATUS_OK[] = "OK";
/** Status string for command error*/
static const char STATUS_ERROR[] = "Error";


/**
 *	\brief				Initializes sender for using remote DeviceHive server.
 *	\param[in]	ip		Remote server IP.
 *	\param[in]	port	Remote server port.
 */
void dhsender_init(ip_addr_t *ip, int port);

/**
 *	\brief				Send command response.
 *	\param[in]	id		Command id that responce should be sent.
 *	\param[in]	status	Null terminated string with command status.
 *	\param[in]	result	Null terminated string with command result.
 */
void dhsender_response(int id, const char *status, const char *result);

/**
 *	\brief					Send notification.
 *	\param[in]	name		Null terminated string with notification name.
 *	\param[in]	parameters	Null terminated string with notification parameters.
 */
void dhsender_notification(const char *name, const char *parameters);

/**
 *	\brief				Stops repeat attempts on error.
 */
void dhsender_stop_repeat();

#endif /* DHSENDER_H_ */
