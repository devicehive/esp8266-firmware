/**
 *	\file		httpd.h
 *	\brief		Module for handling uploadable HTTP API calls.
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */
#ifndef _UPLOADABLE_API_H_
#define _UPLOADABLE_API_H_

#include "httpd.h"

/**
 *	\brief						Handle rest request.
 *	\param[in]	path			Url path.
 *	\param[in]	key				Access key for API which was given in request.
 *	\param[in]	content_in		Request body, typically json.
 *	\return						One of httpd statuses.
 */

HTTP_RESPONSE_STATUS uploadable_api_handle(const char *path, const char *key,
		HTTP_CONTENT *content_in, HTTP_ANSWER *answer);

#endif /* _UPLOADABLE_API_H_ */
