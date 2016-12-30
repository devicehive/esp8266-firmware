/*
 * dhzc_web.h
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include "httpd.h"

#include <ets_sys.h>
#include <osapi.h>
#include <user_interface.h>
#include "httpd.h"
#include "dhdebug.h"
#include "dhesperrors.h"
#include "dhsettings.h"
#include "dhzc_pages.h"
#include "dhzc_post.h"

#define WEB_CONF_HOST "devicehive.config"
#define RECONFIGURE_DELAY_MS 5000

LOCAL unsigned int mConfigured = 0;
LOCAL os_timer_t mReconfigureTimer;

LOCAL void ICACHE_FLASH_ATTR system_reconfigure(void *arg) {
	dhdebug("Rebooting...");
	system_restart();
}

LOCAL HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR check_if_configured(
		HTTP_ANSWER *answer) {
	if(mConfigured) {
		answer->content.data = dhzc_pages_ok(&answer->content.len);
		if(answer->content.data == 0) {
			return HRCS_INTERNAL_ERROR;
		}
		dhdebug("Already configured");
		return HRCS_ANSWERED_HTML;
	}
	return HRCS_NOT_FINISHED;
}

LOCAL HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR get_cb(const char *path,
		const char *key, HTTP_CONTENT *content_in, HTTP_ANSWER *answer) {
	HTTP_RESPONSE_STATUS res = check_if_configured(answer);
	if(res != HRCS_NOT_FINISHED)
		return res;
	if(path[0] == '/') {
		if(path[1] == 0) {
			answer->content.data = dhzc_pages_form(&answer->content.len);
			if(answer->content.data == 0) {
				return HRCS_INTERNAL_ERROR;
			}
			return HRCS_ANSWERED_HTML;
		}
	}
	return HRCS_NOT_FOUND;
}

LOCAL HTTP_RESPONSE_STATUS ICACHE_FLASH_ATTR post_cb(const char *path,
		const char *key, HTTP_CONTENT *content_in, HTTP_ANSWER *answer) {
	HTTP_RESPONSE_STATUS res = check_if_configured(answer);
	if(res != HRCS_NOT_FINISHED)
		return res;
	dhdebug("got POST with settings len %u", content_in->len);
	char *parse_res = dhzc_post_parse(content_in->data, content_in->len);
	if(parse_res) {
		answer->content.data = dhzc_pages_error(parse_res, &answer->content.len);
	} else {
		if(dhsettings_commit() == 0) {
			return HRCS_INTERNAL_ERROR;
		} else {
			answer->content.data = dhzc_pages_ok(&answer->content.len);
			if(answer->content.data == 0) {
				dhdebug("Generate OK page fail");
				return HRCS_INTERNAL_ERROR;
			} else {
				dhdebug("Configuration was written. Will be rebooted in %d ms", RECONFIGURE_DELAY_MS);
				os_timer_disarm(&mReconfigureTimer);
				os_timer_setfn(&mReconfigureTimer, (os_timer_func_t *)system_reconfigure, NULL);
				os_timer_arm(&mReconfigureTimer, RECONFIGURE_DELAY_MS, 0);
				mConfigured = 1;
			}
		}
	}
	return HRCS_ANSWERED_HTML;
}

void ICACHE_FLASH_ATTR dhzc_web_init() {
	httpd_redirect(WEB_CONF_HOST);
	httpd_init(get_cb, post_cb);
}
