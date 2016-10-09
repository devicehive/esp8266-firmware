/**
 *	\file		dhsender_queue.h
 *	\brief		Module for storing request that should be sent
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHSENDER_QUEUE_H_
#define _DHSENDER_QUEUE_H_

#include <stdarg.h>
#include "dhrequest.h"
#include "dhsender_data.h"

/**
 *	\brief							Add new request for dhsender in queue.
 *	\param[in]	type				Request type, see REQUEST_TYPE enum.
 *	\param[in]	notification_type	If it is notification, this parameter should contain notification type. Ignore for responses.
 *	\param[in]	data_type			Type of data that passed to function.
 *	\param[in]	id					CommandId for response. Ignore for notifications.
 *	\param[in]	ap					Request data.
 *	\return							Non zero value on success, zero on error.
 */
int dhsender_queue_add(REQUEST_TYPE type, REQUEST_NOTIFICATION_TYPE notification_type, REQUEST_DATA_TYPE data_type, unsigned int id, va_list ap);

/**
 *	\brief						Take one item from queue.
 *	\details					Taken item will be deleted from queue.
 *	\param[out]	out				Pointer to HTTP_REQUEST that should be created.
 *	\param[out]	is_notification	Pointer to variable that will receive non zero value if taken request is notification, zero value otherwise.
 *	\return						Non zero value on success, zero on error.
 */
int dhsender_queue_take(HTTP_REQUEST *out, unsigned int *is_notification);

/**
 *	\brief				Getting current queue size.
 *	\return				Number of item currently in queue.
 */
unsigned int dhsender_queue_length();

/**
 *	\brief				Initialize queue.
 */
void dhsender_queue_init();

#endif /* _DHSENDER_QUEUE_H_ */
