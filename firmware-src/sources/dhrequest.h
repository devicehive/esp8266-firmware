/**
 *	\file		dhrequest.h
 *	\brief		Generating HTTP request for DeviceHive server.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 *	\details 	All generated request will stored in dynamic memory. When request no longer need in have to be free with dhrequest_free().
 */

#ifndef USER_DHREQUEST_H_
#define USER_DHREQUEST_H_

/** HTTP requests linked list struct. */
typedef struct __attribute__((packed)) HTTP_REQUEST HTTP_REQUEST;
struct __attribute__((packed)) HTTP_REQUEST{
	HTTP_REQUEST *next; 	//< pointer to next HTTP_REQUEST.
	unsigned int len;		//< request data size in bytes.
	char body[];			//< request data.
};

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
 *	\brief	Create register request.
 *	\return	Pointer created request or NULL if no memory.
 */
HTTP_REQUEST *dhrequest_create_register();

/**
 *	\brief					Create poll request.
 *	\param[in]	timestamp	Pointer to null terminated string with timestamp that will be used in poll request.
 *	\return					Pointer created request or NULL if no memory.
 */
HTTP_REQUEST *dhrequest_create_poll(const char *timestamp);

/**
 *	\brief					Create update request.
 *	\param[in]	commandId	Id of command for update.
 *	\param[in]	status		Null terminated string with command status.
 *	\param[in]	result		Null terminated string with command result.
 *	\return					Pointer created request or NULL if no memory.
 */
HTTP_REQUEST *dhrequest_create_update(int commandId, const char *status, const char *result);

/**
 *	\brief					Create info request.
 *	\return					Pointer created request or NULL if no memory.
 */
HTTP_REQUEST *dhrequest_create_info();

/**
 *	\brief					Create notification request.
 *	\param[in]	name		Notification name.
 *	\param[in]	parameters	Notification parameters.
 *	\return					Pointer created request or NULL if no memory.
 */
HTTP_REQUEST *dhrequest_create_notification(const char *name, const char *parameters);

/**
 *	\brief					Update poll request.
 *	\details				This function updates timestamp in poll request. If timestamp string length is different, request will be recreated, otherwise original will be reused.
 *	\param[in]	old			Pointer for request that should be updated.
 *	\param[in]	timestamp	Null terminated string with new timestamp.
 *	\return					Pointer to updated request or NULL if no memory.
 */
HTTP_REQUEST *dhrequest_update_poll(HTTP_REQUEST *old, const char *timestamp);

/**
 *	\brief					Free request.
 *	\param[in]	request		Pointer for request.
 */
void dhrequest_free(HTTP_REQUEST *request);

/**
 *	\brief					Util function to find out response code in HTTP response.
 *	\param[in]	data		Pointer to HTTP response data.
 *	\param[in]	len			HTTP response data length.
 */
const char *dhrequest_find_http_responce_code(const char *data, unsigned short len);

#endif /* USER_DHREQUEST_H_ */
