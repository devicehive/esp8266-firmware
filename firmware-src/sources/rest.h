/**
 *	\file		rest.h
 *	\brief		Local RESTful service
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef _REST_H_
#define _REST_H_

#include "httpd.h"

/**
 *	\brief						Handle rest request.
 *	\param[in]	path			Url path.
 *	\param[in]	key				Access key for API which was given in request.
 *	\param[in]	content_in		Request body, typically json.
 *	\param[out]	content_out		Data for response
 *	\return						One of httpd statuses.
 */

HTTP_RESPONSE_STATUS rest_handle(const char *path, const char *key,
		HTTP_CONTENT *content_in, HTTP_CONTENT *content_out);

#endif /* _REST_H_ */
