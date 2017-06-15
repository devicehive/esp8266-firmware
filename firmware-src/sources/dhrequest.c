/*
 * dhrequest.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for creating HTTP requests for DeviceHive server
 *
 */
#include "dhrequest.h"
#include "user_config.h"
#include "dhdebug.h"
#include "snprintf.h"
#include "dhsettings.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <ets_forward.h>
#include <mem.h>

RO_DATA char HTTP_INFO_REQUEST_PATTERN[] =
		"GET %s/info HTTP/1.1\r\n"
		"Host: %s\r\n\r\n";

RO_DATA char HTTP_WS_REQUEST_PATTERN[] =
		"GET %s/client HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
		"Origin: %s\r\n"
		"Sec-WebSocket-Protocol: chat, superchat\r\n"
		"Sec-WebSocket-Version: 13\r\n\r\n";


const char * ICACHE_FLASH_ATTR dhrequest_parse_url(const char *url, char *host, int *port) {
	int i;
	const char *fr = url;
	while (*fr != ':') {
		fr++;
		if(*fr == 0) {
			return NULL;
		}
	}
	for(i = 0; i < 2; i++) {
		fr++;
		if(*fr != '/')
			return NULL;
	}
	fr++;
	i = 0;
	while (*fr != '/' && *fr != ':' && *fr != 0 &&  i < (DHREQUEST_HOST_MAX_BUF_LEN - 1))
		host[i++] = *fr++;
	host[i] = 0;

	// read port if present
	int p = 0;
	if(*fr == ':') {
		unsigned char d;
		fr++;
		while ((d = *fr - 0x30) < 10) {
			fr++;
			p = p * 10 + d;
			if(p > 0xFFFF)
				break;
		}
	}
	if(p && p < 0xFFFF)
		*port = p;
	else if(os_strncmp(url, "https", 5) == 0 || os_strncmp(url, "wss", 3) == 0)
		*port = 443; // HTTPS default port
	else
		*port = 80; //HTTP default port
	while (*fr != '/' && *fr != 0)
		fr++;
	return *fr ? fr : "/";
}

HTTP_REQUEST * ICACHE_FLASH_ATTR dhrequest_create_info(const char *api) {
	static char sbuf[sizeof(unsigned int) + sizeof(HTTP_INFO_REQUEST_PATTERN) + DHSETTINGS_SERVER_MAX_LENGTH];
	HTTP_REQUEST *buf = (HTTP_REQUEST *)sbuf;
	char host[DHREQUEST_HOST_MAX_BUF_LEN];
	int port;
	const char *path = dhrequest_parse_url(api, host, &port);
	if(path == NULL)
		return NULL;
	buf->len = snprintf(buf->data, sizeof(sbuf) - sizeof(unsigned int), HTTP_INFO_REQUEST_PATTERN, path, host);
	dhdebug("Info request created, %d/%d", buf->len + 1, sizeof(sbuf) - sizeof(unsigned int));
	return buf;
}

HTTP_REQUEST * ICACHE_FLASH_ATTR dhrequest_create_wsrequest(const char *api, const char *url) {
	static char sbuf[sizeof(unsigned int) + sizeof(HTTP_WS_REQUEST_PATTERN) + 2 * DHSETTINGS_SERVER_MAX_LENGTH];
	HTTP_REQUEST *buf = (HTTP_REQUEST *)sbuf;
	char host[DHREQUEST_HOST_MAX_BUF_LEN];
	int port;
	const char *path = dhrequest_parse_url(url, host, &port);
	if(path == NULL)
		return NULL;
	buf->len = snprintf(buf->data, sizeof(sbuf) - sizeof(unsigned int), HTTP_WS_REQUEST_PATTERN, path, host, api);
	dhdebug("WS request created, %d/%d", buf->len + 1, sizeof(sbuf) - sizeof(unsigned int));
	return buf;
}
