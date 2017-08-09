/**
 *	\file		dhrequest.h
 *	\brief		Generating HTTP requests for DeviceHive server.
 *	\author		Nikolay Khabarov
 *	\date		2017
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHREQUEST_H_
#define _DHREQUEST_H_

#include "dhsettings.h"
#include "user_config.h"

/** Maximum length of host in urls */
#define DHREQUEST_HOST_MAX_BUF_LEN 256

/** Structure for HTTP request */
typedef struct {
	unsigned int len;
	char data[];
} HTTP_REQUEST;

/**
 *	\brief					Extract host, port, path from URL
 *	\param[in]	url			URL to parse.
 *	\param[out]	host		Pointer with bugger where to store host. Should be at least DHREQUEST_HOST_MAX_BUF_LEN bytes. Cannot extract more then DHREQUEST_HOST_MAX_BUF_LEN - 1 chars.
 *	\param[out]	port		Pointer to int value to store port.
 *	\return					Pointer to url's path or NULL on error.
 */
const char *dhrequest_parse_url(const char *url, char *host, int *port);

/**
 *	\brief					Create info request.
 *	\details				Return valuse is single instance. Each call return pointer to the same buffer.
 *	\param[in]	api			DeviceHive API url.
 *	\return					Pointer to HTTP_REQUEST struct.
 */
HTTP_REQUEST *dhrequest_create_info(const char *api);

/**
 *	\brief					Create WebSocket upgrade request.
 *	\details				Return value is single instance. Each call return pointer to the same buffer.
 *	\param[in]	api			DeviceHive API url.
 *	\param[in]	url			WebSocket API url.
 *	\return					Pointer to HTTP_REQUEST struct.
 */
HTTP_REQUEST *dhrequest_create_wsrequest(const char *api, const char *url);

#endif /* _DHREQUEST_H_ */
