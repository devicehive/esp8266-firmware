/*
 * rest.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <c_types.h>
#include <osapi.h>
#include "rest.h"
#include "dhrequest.h"

HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR rest_handle(const char *path, const char *key,
		HTTP_CONTENT *content_in, HTTP_CONTENT *content_out) {
	if(dhrequest_current_accesskey()[0]) {
		if(key == 0) {
			return HRCS_UNAUTHORIZED;
		}
		if(os_strcmp(key,dhrequest_current_accesskey())) {
			return HRCS_UNAUTHORIZED;
		}
	}

	// TODO
	return HRCS_NOT_IMPLEMENTED;
}
