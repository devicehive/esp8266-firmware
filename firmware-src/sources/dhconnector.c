/*
 * dhconnector.cpp
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for connecting to remote DeviceHive server
 *
 */
#include "dhconnector.h"
#include "dhdebug.h"
#include "dhmem.h"
#include "dhutils.h"
#include "user_config.h"
#include "snprintf.h"
#include "dhstatistic.h"
#include "mdnsd.h"
#include "dhconnector_websocket.h"
#include "dhesperrors.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <espconn.h>
#include <json/jsonparse.h>
#include <ets_forward.h>

LOCAL CONNECTION_STATE mConnectionState;
LOCAL struct espconn mDHConnector;
LOCAL os_timer_t mRetryTimer;
LOCAL unsigned char mNeedRecover = 0;
LOCAL char mWSUrl[DHSETTINGS_SERVER_MAX_LENGTH];

LOCAL void set_state(CONNECTION_STATE state);

LOCAL void retry(void *arg) {
	set_state(mConnectionState);
}

LOCAL void arm_repeat_timer(unsigned int ms) {
	os_timer_disarm(&mRetryTimer);
	os_timer_setfn(&mRetryTimer, (os_timer_func_t *)retry, NULL);
	os_timer_arm(&mRetryTimer, ms, 0);
}

LOCAL void ICACHE_FLASH_ATTR network_error_cb(void *arg, sint8 err) {
	dhesperrors_espconn_result("Connector error occurred:", err);
	mConnectionState = CS_DISCONNECT;
	arm_repeat_timer(RETRY_CONNECTION_INTERVAL_MS);
	dhstat_got_network_error();
}

LOCAL void ICACHE_FLASH_ATTR parse_json(struct jsonparse_state *jparser) {
	int type;
	while (jparser->pos < jparser->len) {
		type = jsonparse_next(jparser);
		if(type == JSON_TYPE_PAIR_NAME) {
			if(jsonparse_strcmp_value(jparser, "webSocketServerUrl") == 0) {
				jsonparse_next(jparser);
				if(jsonparse_next(jparser) != JSON_TYPE_ERROR) {
					jsonparse_copy_value(jparser, mWSUrl, sizeof(mWSUrl));
					dhdebug("WebSocker URL received %s", mWSUrl);
				}
				break;
			}
		} else if(type == JSON_TYPE_ERROR) {
			if(jparser->pos > 0 && jparser->len - jparser->pos >= 3) { // fix issue with parsing null value
				if(os_strncmp(&jparser->json[jparser->pos - 1], "null", 4) == 0) {
					jparser->pos += 3;
					jparser->vtype = JSON_TYPE_NULL;
					continue;
				}
			}
			break;
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR ws_error(void) {
	dhstat_got_server_error();
	// close connection and restart everything on error
	espconn_disconnect(&mDHConnector);
}

LOCAL void ICACHE_FLASH_ATTR ws_send(const char *data, unsigned int len) {
	if(mConnectionState != CS_OPERATE)
		return;
	if(espconn_send(&mDHConnector, (uint8 *)data, len) != ESPCONN_OK)
		ws_error();
	else
		dhstat_add_bytes_sent(len);
}

LOCAL void ICACHE_FLASH_ATTR network_recv_cb(void *arg, char *data, unsigned short len) {
	dhstat_add_bytes_received(len);
	if(mConnectionState == CS_OPERATE) {
		dhconnector_websocket_parse(data, len);
		return;
	}
	const char *rc = find_http_responce_code(data, len);
	if(rc) { // HTTP
		if(rc[0] == '1' && rc[1] == '0' && rc[2] == '1' && mConnectionState == CS_WEBSOCKET) { // HTTP responce code 101 - Switching Protocols
			set_state(CS_OPERATE);
			dhdebug("WebSocket connection is established");
			dhconnector_websocket_start(ws_send, ws_error);
			// do not disconnect
			return;
		} else if(*rc == '2' && mConnectionState == CS_GETINFO) { // HTTP responce code 2xx - Success
			if(os_strstr(data, (char *) "\r\n\r\n")) {
				int deep = 0;
				unsigned int pos = 0;
				unsigned int jsonstart = 0;
				while (pos < len) {
					if(data[pos] == '{') {
						if(deep == 0)
							jsonstart = pos;
						deep++;
					} else if(data[pos] == '}') {
						deep--;
						if(deep == 0) {
							struct jsonparse_state jparser;
							jsonparse_setup(&jparser, &data[jsonstart],
									pos - jsonstart + 1);
							parse_json(&jparser);
						}
					}
					pos++;
				}
			}
		} else {
			mConnectionState = CS_DISCONNECT;
			dhdebug("Connector HTTP response bad status %c%c%c", rc[0],rc[1],rc[2]);
			dhdebug_ram(data);
			dhdebug("--------------------------------------");
			dhstat_got_server_error();
		}
	} else {
		mConnectionState = CS_DISCONNECT;
		dhdebug("Connector HTTP magic number is wrong");
		dhstat_got_server_error();
	}
	espconn_disconnect(&mDHConnector);
}

LOCAL void network_connect_cb(void *arg) {
	HTTP_REQUEST *request = 0;
	uint32_t keepalive;
	espconn_set_opt(&mDHConnector, ESPCONN_KEEPALIVE);
	//set keepalive: 120s = 90 + 10 * 3
	keepalive = 90;
	espconn_set_keepalive(&mDHConnector, ESPCONN_KEEPIDLE, &keepalive);
	keepalive = 10;
	espconn_set_keepalive(&mDHConnector, ESPCONN_KEEPINTVL, &keepalive);
	keepalive = 3;
	espconn_set_keepalive(&mDHConnector, ESPCONN_KEEPCNT, &keepalive);
	switch (mConnectionState) {
	case CS_GETINFO:
		request = dhrequest_create_info(dhsettings_get_devicehive_server());
		dhdebug("Send info request...");
		break;
	case CS_WEBSOCKET:
		request = dhrequest_create_wsrequest(dhsettings_get_devicehive_server(), mWSUrl);
		dhdebug("Send web socket upgrade request...");
		break;
	/* TODO case CS_POLL:
	case CS_CUSTOM:
		request = custom_firmware_request();
		if(request) {
			mConnectionState = CS_CUSTOM;
		} else {
			request = &mPollRequest;
			dhdebug("Send poll request...");
		}
		break;*/
	default:
		dhdebug("ASSERT: networkConnectCb wrong state %d", mConnectionState);
	}
	int res;
	if( (res = espconn_send(&mDHConnector, (uint8_t*)request->data, request->len)) != ESPCONN_OK) {
		mConnectionState = CS_DISCONNECT;
		dhesperrors_espconn_result("network_connect_cb failed:", res);
		espconn_disconnect(&mDHConnector);
	} else {
		dhstat_add_bytes_sent(request->len);
	}
}

LOCAL void network_disconnect_cb(void *arg) {
	switch(mConnectionState) {
	case CS_GETINFO:
		set_state(CS_WEBSOCKET);
		break;
	case CS_DISCONNECT:
	case CS_WEBSOCKET:
	case CS_OPERATE:
		mConnectionState = CS_DISCONNECT;
		arm_repeat_timer(RETRY_CONNECTION_INTERVAL_MS);
		break;
/* TODO case CS_CUSTOM:
		if(dhterminal_is_in_use()) {
			dhdebug("Terminal is in use, no deep sleep");
			arm_repeat_timer(CUSTOM_NOTIFICATION_INTERVAL_MS);
		} else {
			system_deep_sleep(CUSTOM_NOTIFICATION_INTERVAL_MS * 1000);
			// after deep sleep chip will be rebooted
		}
		break;
*/
	default:
		dhdebug("ASSERT: networkDisconnectCb wrong state %d", mConnectionState);
	}
}

LOCAL void ICACHE_FLASH_ATTR dhconnector_init_connection(ip_addr_t *ip) {
	os_memcpy(mDHConnector.proto.tcp->remote_ip, &ip->addr, sizeof(ip->addr));
	espconn_regist_connectcb(&mDHConnector, network_connect_cb);
	espconn_regist_recvcb(&mDHConnector, network_recv_cb);
	espconn_regist_reconcb(&mDHConnector, network_error_cb);
	espconn_regist_disconcb(&mDHConnector, network_disconnect_cb);
}

LOCAL void ICACHE_FLASH_ATTR resolve_cb(const char *name, ip_addr_t *ip, void *arg) {
	if(ip == NULL) {
		dhdebug("Resolve %s failed. Trying again...", name);
		mConnectionState = CS_DISCONNECT;
		arm_repeat_timer(RETRY_CONNECTION_INTERVAL_MS);
		dhstat_got_network_error();
		return;
	}
	unsigned char *bip = (unsigned char *) ip;
	dhdebug("Host %s ip: %d.%d.%d.%d, using port %d", name, bip[0], bip[1], bip[2], bip[3], mDHConnector.proto.tcp->remote_port);

	dhconnector_init_connection(ip);
	if(mConnectionState == CS_DISCONNECT)
		set_state(CS_GETINFO);
	else if(mConnectionState == CS_RESOLVEWEBSOCKET)
		set_state(CS_WEBSOCKET);
	else
		dhdebug("ASSERT: Wrong state on resolve");
}

LOCAL void ICACHE_FLASH_ATTR start_resolve_dh_server(const char *server) {
	static ip_addr_t ip;
	char host[DHREQUEST_HOST_MAX_BUF_LEN];
	if(dhrequest_parse_url(server, host, &mDHConnector.proto.tcp->remote_port)) {
		dhdebug("Resolving %s", host);
		err_t r = espconn_gethostbyname(&mDHConnector, host, &ip, resolve_cb);
		if(r == ESPCONN_OK) {
			resolve_cb(host, &ip, NULL);
		} else if(r != ESPCONN_INPROGRESS) {
			dhesperrors_espconn_result("Resolving failed:", r);
			arm_repeat_timer(RETRY_CONNECTION_INTERVAL_MS);
		}
	} else {
		dhdebug("Can not find scheme in server url. Server connectivity is disabled.");
	}
}

void ICACHE_FLASH_ATTR dhmem_unblock_cb(void) {
	if(mNeedRecover) {
		set_state(mConnectionState);
		mNeedRecover = 0;
	}
}

LOCAL void ICACHE_FLASH_ATTR set_state(CONNECTION_STATE state) {
	mConnectionState = state;
	if(state != CS_DISCONNECT) {
		if(dhmem_isblock()) {
			mNeedRecover = 1;
			return;
		}
	}
	switch(state) {
	case CS_DISCONNECT:
		if(os_strncmp(dhsettings_get_devicehive_server(), "ws://", 5) == 0 ||
				os_strncmp(dhsettings_get_devicehive_server(), "wss://", 6) == 0 ) {
			snprintf(mWSUrl, sizeof(mWSUrl), "%s", dhsettings_get_devicehive_server());
			mConnectionState = CS_RESOLVEWEBSOCKET;
			start_resolve_dh_server(mWSUrl);
		} else {
			mWSUrl[0] = 0;
			start_resolve_dh_server(dhsettings_get_devicehive_server());
		}
		break;
	case CS_RESOLVEWEBSOCKET:
		if(mWSUrl[0] == 0) {
			dhdebug("Failed to get WebSocket URL");
			set_state(CS_DISCONNECT);
		}
		start_resolve_dh_server(mWSUrl);
		break;
	case CS_GETINFO:
	case CS_WEBSOCKET:
	{
		const sint8 cr = espconn_connect(&mDHConnector);
		if(cr == ESPCONN_ISCONN)
			return;
		if(cr != ESPCONN_OK) {
			dhesperrors_espconn_result("Connector espconn_connect failed:", cr);
			arm_repeat_timer(RETRY_CONNECTION_INTERVAL_MS);
		}
		break;
	}
	case CS_OPERATE:
		break;
	default:
		dhdebug("ASSERT: set_state wrong state %d", mConnectionState);
	}
}

LOCAL void ICACHE_FLASH_ATTR wifi_state_cb(System_Event_t *event) {
	if(event->event == EVENT_STAMODE_GOT_IP) {
		if(event->event_info.got_ip.ip.addr != 0) {
			const unsigned char * const bip = (unsigned char *)&event->event_info.got_ip.ip;
			dhdebug("WiFi connected, ip: %d.%d.%d.%d", bip[0], bip[1], bip[2], bip[3]);
			mConnectionState = CS_DISCONNECT;
			arm_repeat_timer(DHREQUEST_PAUSE_MS);

			if(dhsettings_get_devicehive_deviceid()[0]) {
				mdnsd_start(dhsettings_get_devicehive_deviceid(), event->event_info.got_ip.ip.addr);
			}
		} else {
			dhdebug("ERROR: WiFi reports STAMODE_GOT_IP, but no actual ip found");
		}
	} else if(event->event == EVENT_STAMODE_DISCONNECTED) {
		os_timer_disarm(&mRetryTimer);
		dhesperrors_disconnect_reason("WiFi disconnected", event->event_info.disconnected.reason);
		dhstat_got_wifi_lost();
		mdnsd_stop();
	} else {
		dhesperrors_wifi_state("WiFi event", event->event);
	}
}

void ICACHE_FLASH_ATTR dhconnector_init(void) {
	mConnectionState = CS_DISCONNECT;

	wifi_set_opmode(STATION_MODE);
	wifi_station_set_auto_connect(1);
	wifi_station_set_reconnect_policy(true);
	struct station_config stationConfig;
	wifi_station_get_config(&stationConfig);
	wifi_set_phy_mode(PHY_MODE_11N);
	os_memset(stationConfig.ssid, 0, sizeof(stationConfig.ssid));
	os_memset(stationConfig.password, 0, sizeof(stationConfig.password));
	snprintf((char*)stationConfig.ssid, sizeof(stationConfig.ssid), "%s", dhsettings_get_wifi_ssid());
	snprintf((char*)stationConfig.password, sizeof(stationConfig.password), "%s", dhsettings_get_wifi_password());
	if(dhsettings_get_devicehive_deviceid()[0]) {
		char hostname[33];
		// limit length to 32 chars due to the sdk limit
		snprintf(hostname, sizeof(hostname), "%s", dhsettings_get_devicehive_deviceid());
		wifi_station_set_hostname(hostname);
	}
	wifi_station_set_config(&stationConfig);

	static esp_tcp tcp;
	os_memset(&tcp, 0, sizeof(tcp));
	os_memset(&mDHConnector, 0, sizeof(mDHConnector));
	mDHConnector.type = ESPCONN_TCP;
	mDHConnector.state = ESPCONN_NONE;
	mDHConnector.proto.tcp = &tcp;
	mDHConnector.proto.tcp->local_port = espconn_port();

	wifi_set_event_handler_cb(wifi_state_cb);
}

CONNECTION_STATE ICACHE_FLASH_ATTR dhconnector_get_state(void) {
	return mConnectionState;
}
