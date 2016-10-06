/**
 *	\file		dhrequest.h
 *	\brief		Generating HTTP request for DeviceHive server.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 *	\details 	All generated request will stored in dynamic memory. When request no longer need in have to be free with dhrequest_free().
 */

#ifndef _DHREQUEST_H_
#define _DHREQUEST_H_

#include "dhsettings.h"
#include "user_config.h"

/** HTTP requests theoretical max size */
#define HTTP_REQUEST_MIN_ALLOWED_PAYLOAD 2*INTERFACES_BUF_SIZE + 943 // 943 for json formatting and some extra text fields, we use exactly this value to pad to KB.
#define HTTP_REQUEST_MAX_SIZE (DHSETTINGS_SERVER_MAX_LENGTH + DHSETTINGS_ACCESSKEY_MAX_LENGTH + HTTP_REQUEST_MIN_ALLOWED_PAYLOAD)
typedef struct {
	unsigned int len;
	char data[HTTP_REQUEST_MAX_SIZE];
} HTTP_REQUEST;

/**
 *	\brief	Load DeviceHive server credentials from storage.
 */
void dhrequest_load_settings();

/**
 *	\brief	Get current DeviceHive server URL.
 *	\return	Pointer to null terminated buffer with DeviceHive server URL.
 */
const char *dhrequest_current_server();

/**
 *	\brief	Get current DeviceHive DeviceId.
 *	\return	Pointer to null terminated buffer with DeviceHive DeviceId.
 */
const char *dhrequest_current_deviceid();

/**
 *	\brief			Create register request.
 *	\param[out]	buf	Buffer where request should be created.
 */
void dhrequest_create_register(HTTP_REQUEST *buf);

/**
 *	\brief					Create poll request.
 *	\param[out]	buf			Buffer where request should be created.
 *	\param[in]	timestamp	Pointer to null terminated string with timestamp that will be used in poll request.
 */
void dhrequest_create_poll(HTTP_REQUEST *buf, const char *timestamp);

/**
 *	\brief					Create update request.
 *	\param[out]	buf			Buffer where request should be created.
 *	\param[in]	commandId	Id of command for update.
 *	\param[in]	status		Null terminated string with command status.
 *	\param[in]	result		Null terminated string with command result.
 *	\return					Pointer created request or NULL if no memory.
 */
void dhrequest_create_update(HTTP_REQUEST *buf, unsigned int commandId, const char *status, const char *result);

/**
 *	\brief					Create info request.
 *	\param[out]	buf			Buffer where request should be created.
 */
void dhrequest_create_info(HTTP_REQUEST *buf);

/**
 *	\brief					Create notification request.
 *	\param[out]	buf			Buffer where request should be created.
 *	\param[in]	name		Notification name.
 *	\param[in]	parameters	Notification parameters.
 */
void dhrequest_create_notification(HTTP_REQUEST *buf, const char *name, const char *parameters);

/**
 *	\brief					Update poll request.
 *	\details				This function updates timestamp in poll request. If timestamp string length is different, request will be recreated, otherwise original will be reused.
 *	\param[in]	old			Pointer for request that should be updated.
 *	\param[in]	timestamp	Null terminated string with new timestamp.
 */
void dhrequest_update_poll(HTTP_REQUEST *old, const char *timestamp);

#endif /* _DHREQUEST_H_ */
