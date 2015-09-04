/*
 * dhap_httpd.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <user_interface.h>
#include <espconn.h>
#include <mem.h>
#include "dhap_httpd.h"
#include "dhdebug.h"
#include "dhap_pages.h"
#include "dhap_post.h"
#include "dhesperrors.h"
#include "dhsettings.h"

#define MAX_CONNECTIONS 8

#define HTTPD_PORT 80

#define HTTPD_URL "devicehive.config"

LOCAL unsigned int mConnected = 0;
LOCAL unsigned int mConfigured = 0;
LOCAL struct espconn *mCurrentPost = 0;
LOCAL char *mPostBuf = 0;
LOCAL unsigned int mPostBufPos = 0;

#define RECONFIGURE_DELAY_MS 5000
LOCAL os_timer_t mReconfigureTimer;
LOCAL void ICACHE_FLASH_ATTR system_reconfigure(void *arg) {
	dhdebug("Rebooting...");
	system_restart();
}

LOCAL void ICACHE_FLASH_ATTR check_send_res(sint8 res) {
	if(res)
		dhesperrors_espconn_result("espconn_send returned: %d", res);
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_disconnect_cb(void *arg) {
	mConnected--;
	dhdebug("Client disconnected, %u left", mConnected);
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_sent_cb(void *arg) {
	struct espconn *conn = arg;
	espconn_disconnect(conn);
	dhdebug("Data sent");
}

LOCAL void ICACHE_FLASH_ATTR receive_post(const char *data, unsigned short len, char * internal, unsigned int internalsize) {
	const unsigned int POST_BUF_SIZE = 2048;
	const char content_length[] = "Content-Length:";
	const char sp[] = "\r\n\r\n";
	if(mPostBuf == 0) {
		mPostBuf = (char*)os_malloc(POST_BUF_SIZE);
		if(mPostBuf == 0) {
			check_send_res(espconn_send(mCurrentPost, internal, internalsize));
			mCurrentPost = 0;
			return;
		}
	}
	if(len > POST_BUF_SIZE - mPostBufPos) {
		check_send_res(espconn_send(mCurrentPost, internal, internalsize));
		mCurrentPost = 0;
		return;
	}
	os_memcpy(&mPostBuf[mPostBufPos], data, len);
	mPostBufPos += len;
	const int to = (int)mPostBufPos - sizeof(content_length) + 1;
	int i;
	unsigned int cont_len;
	for(i = 0; i < to; i++) {
		if(os_strncmp(&mPostBuf[i], content_length, sizeof(content_length) - 1) == 0) {
			i += sizeof(content_length) - 1;
			while(mPostBuf[i] == ' ')
				i++;
			if(strToUInt(&mPostBuf[i], &cont_len)) {
				for(; i < mPostBufPos; i++) {
					if(os_strncmp(&mPostBuf[i], sp, sizeof(sp) - 1) == 0) {
						i += sizeof(sp) - 1;
						if(cont_len <= mPostBufPos - i) {
							dhdebug("POST len %u/%u", cont_len, mPostBufPos - i);
							char *res = dhap_post_parse(&mPostBuf[i], mPostBufPos - i);
							unsigned int rlen;
							if(res) {
								res = dhap_pages_error(res, &rlen);
							} else {
								if(dhsettings_commit() == 0) {
									res = internal;
									rlen = internalsize;
								} else {
									res = dhap_pages_ok(&rlen);
									if(res == 0) {
										dhdebug("Generate OK page fail");
										res = internal;
										rlen = internalsize;
									} else {
										dhdebug("Configuration was written. Will be rebooted in %d ms", RECONFIGURE_DELAY_MS);
										os_timer_disarm(&mReconfigureTimer);
										os_timer_setfn(&mReconfigureTimer, (os_timer_func_t *)system_reconfigure, NULL);
										os_timer_arm(&mReconfigureTimer, RECONFIGURE_DELAY_MS, 0);
										mConfigured = 1;
									}
								}
							}
							dhdebug("Parse post, send result %u bytes", rlen);
							check_send_res(espconn_send(mCurrentPost, res, rlen));
							mCurrentPost = 0;
						}
						return;
					}
				}
			}
			return;
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_recv_cb(void *arg, char *data, unsigned short len) {
	struct espconn *conn = arg;
	const char rootget[] = "GET / ";
	const char get[] = "GET ";
	const char post[] = "POST ";
	const char host[] = "Host:";
	char redirectresponse[] = "HTTP/1.0 302 Moved\r\nContent-Length: 0\r\nLocation: http://"HTTPD_URL"\r\n\r\n";
	char internal[] = "HTTP/1.0 500 No Memory\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNo memory";
	char notfound[] = "HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot found";
	char notimplemented[] = "HTTP/1.0 501 Not Implemented\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n\r\nNot Implemented";
	dhdebug("Received %d bytes", len);
	if(conn == mCurrentPost) {
		receive_post(data, len, internal, sizeof(internal) - 1);
		return;
	}
	unsigned char redirect = 1;
	const int to = (int)len - sizeof(HTTPD_URL) + 1 - sizeof(host) + 1;
	int i;
	for(i = 0; i < to; i++) {
		if(os_strncmp(&data[i], host, sizeof(host) - 1) == 0) {
			i += sizeof(host) - 1;
			while(data[i] == ' ')
				i++;
			if(os_strncmp(&data[i], HTTPD_URL, sizeof(HTTPD_URL) - 1) == 0)
				redirect = 0;
			break;
		}
	}
	if(redirect) {
		dhdebug("Redirect");
		check_send_res(espconn_send(conn, redirectresponse, sizeof(redirectresponse) - 1));
	} else if(len < sizeof(rootget) || len < sizeof(post)) {
		dhdebug("Bad request");
		char error[] = "HTTP/1.0 400 Bad Request\r\nContent-Length:0\r\n\r\n";
		check_send_res(espconn_send(conn, error, sizeof(error)));
	} else if(mConfigured) {
		unsigned int rlen;
		char *res = dhap_pages_ok(&rlen);
		if(res == 0) {
			res = internal;
			rlen = sizeof(internal) - 1;
		}
		dhdebug("Already configured, response %u bytes", rlen);
		check_send_res(espconn_send(conn, res, rlen));
	} else if(os_strncmp(data, post, sizeof(post) - 1) == 0) {
		if(mCurrentPost) {
			espconn_disconnect(mCurrentPost);
			dhdebug("New POST sender, refuse old");
		}
		mCurrentPost = conn;
		mPostBufPos = 0;
		receive_post(data, len, internal, sizeof(internal) - 1);
	} else if(os_strncmp(data, rootget, sizeof(rootget) - 1)) {
		if(os_strncmp(data, get, sizeof(get) - 1) == 0) {
			dhdebug("Wrong path");
			check_send_res(espconn_send(conn, notfound, sizeof(notfound) - 1));
		} else {
			dhdebug("Not implemented");
			check_send_res(espconn_send(conn, notimplemented, sizeof(notimplemented) - 1));
		}
	} else {
		unsigned int rlen;
		char *res = dhap_pages_form(&rlen);
		if(res == 0) {
			res = internal;
			rlen = sizeof(internal) - 1;
		}
		dhdebug("Send form, %u bytes", rlen);
		check_send_res(espconn_send(conn, res, rlen));
	}
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_reconnect_cb(void *arg, sint8 err) {
	mConnected--;
	dhdebug("Connection error occurred %d", err);
}

LOCAL void ICACHE_FLASH_ATTR dhap_httpd_connect_cb(void *arg) {
	struct espconn *conn = arg;
	mConnected++;
	if(mConnected > MAX_CONNECTIONS) {
		espconn_disconnect(conn);
		dhdebug("Refuse connection, already %u connections", mConnected);
		return;
	}
	espconn_regist_recvcb(conn, dhap_httpd_recv_cb);
	espconn_regist_disconcb(conn, dhap_httpd_disconnect_cb);
	espconn_regist_sentcb(conn, dhap_httpd_sent_cb);
	dhdebug("Client connected");
}

void ICACHE_FLASH_ATTR dhap_httpd_init() {
	struct espconn *httpdConn = (struct espconn *)os_zalloc(sizeof(struct espconn ));
	esp_tcp *httpdTcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	httpdConn->type = ESPCONN_TCP;
	httpdConn->state = ESPCONN_NONE;
	httpdTcp->local_port = HTTPD_PORT;
	httpdConn->proto.tcp = httpdTcp;

	espconn_regist_connectcb(httpdConn, dhap_httpd_connect_cb);
	espconn_regist_reconcb(httpdConn, dhap_httpd_reconnect_cb);
	sint8 res = espconn_accept(httpdConn);
	if(res)
		dhdebug("espconn_accept returned: %d", res);
}
