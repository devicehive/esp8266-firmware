/*
 * rest.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include "uploadable_api.h"

#include <c_types.h>
#include <osapi.h>
#include "dhrequest.h"
#include "uploadable_page.h"

HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR uploadable_api_handle(const char *path, const char *key,
		HTTP_CONTENT *content_in, HTTP_ANSWER *answer) {
	static const char flash[] = "/flash/page/";
	answer->content.len = 0;
	if(os_strncmp(path, flash, sizeof(flash) - 1) == 0) {
		if(dhrequest_current_accesskey()[0]) {
			if(key == 0) {
				return HRCS_UNAUTHORIZED;
			}
			if(os_strcmp(key, dhrequest_current_accesskey())) {
				return HRCS_UNAUTHORIZED;
			}
		}
		const char *p = &path[sizeof(flash) - 1];
		int res = 0;
		if(os_strcmp(p, "begin") == 0) {
			if(content_in->len == 0)
				res = uploadable_page_begin();
		} else if (os_strcmp(p, "finish") == 0) {
			if(content_in->len == 0)
				res = uploadable_page_finish();
		} else if (os_strcmp(p, "put") == 0) {
			if(content_in->len)
				res = uploadable_page_put(content_in->data, content_in->len);
		} else {
			return HRCS_NOT_FOUND;
		}
		return res ? HRCS_ANSWERED_PLAIN : HRCS_INTERNAL_ERROR;
	}
	return HRCS_NOT_FOUND;
}
