/*
 * webserver.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <c_types.h>
#include <osapi.h>
#include "webserver.h"
#include "httpd.h"
#include "rest.h"

LOCAL int ICACHE_FLASH_ATTR check_rest(HTTP_RESPONSE_STATUS *res, const char *path,
		const char *key, HTTP_CONTENT *content_in, HTTP_ANSWER *answer) {
	static const char api[] = "/api";
	if(os_strncmp(path, api, sizeof(api) - 1) == 0) {
		const char *p = &path[sizeof(api) - 1];
		if(p[0] == 0) {
			*res = rest_handle(p, key, content_in, answer);
			return 1;
		}else if(p[0] == '/') {
			*res = rest_handle(&p[1], key, content_in, answer);
			return 1;
		}
	}
	return 0;
}

LOCAL HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR get_cb(const char *path,
		const char *key, HTTP_CONTENT *content_in, HTTP_ANSWER *answer) {
	HTTP_RESPONSE_STATUS res;
	if(check_rest(&res, path, key, content_in, answer)) {
		return res;
	}
	static const char hw[] = "Hello, world!";
	answer->content.data = hw;
	answer->content.len = sizeof(hw) - 1;
	return HRCS_ANSWERED_PLAIN;
}

LOCAL HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR post_cb(const char *path,
		const char *key, HTTP_CONTENT *content_in, HTTP_ANSWER *answer) {
	HTTP_RESPONSE_STATUS res;
	if(check_rest(&res, path, key, content_in, answer)) {
		return res;
	}
	return HRCS_NOT_FOUND;
}

void ICACHE_FLASH_ATTR webserver_init() {
	httpd_init(get_cb, post_cb);
}
