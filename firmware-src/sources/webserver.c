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
#include "../pages/pages.h"
#include "uploadable_page.h"
#include "uploadable_api.h"
#include "irom.h"

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
	RO_DATA char default_page[] = "<html>\n\t<head>\n\t\t<meta http-equiv=\"refresh\" content=\"5; url=./help.html\"/>\n\t</head>\n\t<body>\n\t\tPage is not uploaded. Redirecting to <a href=\"./help.html\">the help page...</a>\n\t</body>\n</html>";
	if(check_rest(&res, path, key, content_in, answer)) {
		return res;
	}
	if(path[0]=='/') {
		if(path[1]==0) {
			answer->content.data = uploadable_page_get(&answer->content.len);
			if(answer->content.len == 0) {
				// default page
				answer->content.data = default_page;
				answer->content.len = sizeof(default_page) - 1;
			}
			return HRCS_ANSWERED_HTML;
		}

		int i;
		for(i = 0; i < sizeof(web_pages) / sizeof(WEBPAGE); i++) {
			if(os_strcmp(web_pages[i].path, &path[1]) == 0) {
				answer->content.data = web_pages[i].data;
				answer->content.len = web_pages[i].data_len;
				if(os_strstr(&path[1], ".js"))
					return HRCS_ANSWERED_JS;
				if(os_strstr(&path[1], ".css"))
					return HRCS_ANSWERED_CSS;
				if(os_strstr(&path[1], ".ico"))
					return HRCS_ANSWERED_XICON;
				return HRCS_ANSWERED_HTML;
			}
		}
	}

	return HRCS_NOT_FOUND;
}

LOCAL HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR post_cb(const char *path,
		const char *key, HTTP_CONTENT *content_in, HTTP_ANSWER *answer) {
	HTTP_RESPONSE_STATUS res;
	if(check_rest(&res, path, key, content_in, answer)) {
		return res;
	}
	return uploadable_api_handle(path, key, content_in, answer);
}

void ICACHE_FLASH_ATTR webserver_init() {
	httpd_init(get_cb, post_cb);
}
