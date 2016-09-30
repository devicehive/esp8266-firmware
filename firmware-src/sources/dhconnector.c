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

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <espconn.h>
#include <json/jsonparse.h>
#include "dhrequest.h"
#include "dhconnector.h"
#include "dhsender.h"
#include "dhdebug.h"
#include "dhmem.h"
#include "dhutils.h"
#include "user_config.h"
#include "snprintf.h"
#include "dhstatistic.h"

LOCAL CONNECTION_STATE mConnectionState;
LOCAL struct espconn mDHConnector;
LOCAL dhconnector_command_json_cb mCommandCallback;
LOCAL HTTP_REQUEST mRegisterRequest;
LOCAL HTTP_REQUEST mPollRequest;
LOCAL HTTP_REQUEST mInfoRequest;
LOCAL os_timer_t mRetryTimer;
LOCAL unsigned char mNeedRecover = 0;

LOCAL void ICACHE_FLASH_ATTR set_state(CONNECTION_STATE state);

LOCAL void retry(void *arg) {
	set_state(mConnectionState);
}

LOCAL void arm_repeat_timer(unsigned int ms) {
	os_timer_disarm(&mRetryTimer);
	os_timer_setfn(&mRetryTimer, (os_timer_func_t *)retry, NULL);
	os_timer_arm(&mRetryTimer, ms, 0);
}

LOCAL void network_error_cb(void *arg, sint8 err) {
	dhesperrors_espconn_result("Connector error occurred:", err);
	mConnectionState = CS_DISCONNECT;
	arm_repeat_timer(RETRY_CONNECTION_INTERVAL_MS);
	dhstatistic_inc_network_errors_count();
}

LOCAL void parse_json(struct jsonparse_state *jparser) {
	int type;
	unsigned int id;
	char command[128] = "";
	const char *params;
	int paramslen = 0;
	char timestamp[128] = "";
	while ((type = jsonparse_next(jparser)) != JSON_TYPE_ERROR) {
		if (type == JSON_TYPE_PAIR_NAME) {
			if (jsonparse_strcmp_value(jparser, "serverTimestamp") == 0) {
				jsonparse_next(jparser);
				if (jsonparse_next(jparser) != JSON_TYPE_ERROR) {
					jsonparse_copy_value(jparser, timestamp, sizeof(timestamp));
					dhdebug("Timestamp received %s", timestamp);
					dhrequest_update_poll(&mPollRequest, timestamp);
				}
				break;
			} else if (jsonparse_strcmp_value(jparser, "command") == 0) {
				jsonparse_next(jparser);
				if(jsonparse_next(jparser) != JSON_TYPE_ERROR)
					jsonparse_copy_value(jparser, command, sizeof(command));
			} else if (jsonparse_strcmp_value(jparser, "id") == 0) {
				jsonparse_next(jparser);
				if(jsonparse_next(jparser) != JSON_TYPE_ERROR)
					id = jsonparse_get_value_as_ulong(jparser);
			} else if (jsonparse_strcmp_value(jparser, "parameters") == 0) {
				jsonparse_next(jparser);
				if(jsonparse_next(jparser) != JSON_TYPE_ERROR) {
					// there is an issue with extracting subjson with jparser->vstart or jparser_copy_value
					params = &jparser->json[jparser->pos - 1];
					if(*params == '{') {
						int end = jparser->pos;
						while(end < jparser->len && jparser->json[end] != '}') {
							end++;
						}
						paramslen = end - jparser->pos + 2;
					}
				}
			} else if (jsonparse_strcmp_value(jparser, "timestamp") == 0) {
				jsonparse_next(jparser);
				if(jsonparse_next(jparser) != JSON_TYPE_ERROR)
					jsonparse_copy_value(jparser, timestamp, sizeof(timestamp));
			}
		}
	}
	if (mConnectionState == CS_POLL) {
		if(timestamp[0]) {
			dhdebug("Timestamp received %s", timestamp);
			dhrequest_update_poll(&mPollRequest, timestamp);
		}
		mCommandCallback(id, command, params, paramslen);
	}
}

LOCAL void network_recv_cb(void *arg, char *data, unsigned short len) {
	dhstatistic_add_bytes_received(len);
	const char *rc = find_http_responce_code(data, len);
	if (rc) { // HTTP
		if (*rc == '2') { // HTTP responce code 2xx - Success
			if (mConnectionState == CS_REGISTER) {
				dhdebug("Successfully register");
			} else {
				char *content = (char *) os_strstr(data, (char *) "\r\n\r\n");
				if (content && mConnectionState != CS_CUSTOM) {
					int deep = 0;
					unsigned int pos = 0;
					unsigned int jsonstart = 0;
					while (pos < len) {
						if (data[pos] == '{') {
							if (deep == 0)
								jsonstart = pos;
							deep++;
						} else if (data[pos] == '}') {
							deep--;
							if (deep == 0) {
								struct jsonparse_state jparser;
								jsonparse_setup(&jparser, &data[jsonstart],
										pos - jsonstart + 1);
								parse_json(&jparser);
							}
						}
						pos++;
					}
				}
			}
		} else {
			mConnectionState = CS_DISCONNECT;
			dhdebug("Connector HTTP response bad status %c%c%c", rc[0],rc[1],rc[2]);
			dhdebug(data);
			dhdebug("--------------------------------------");
			dhstatistic_server_errors_count();
		}
	} else {
		mConnectionState = CS_DISCONNECT;
		dhdebug("Connector HTTP magic number is wrong");
		dhstatistic_server_errors_count();
	}
	espconn_disconnect(&mDHConnector);
}

LOCAL void network_connect_cb(void *arg) {
	HTTP_REQUEST *request;
	switch(mConnectionState) {
	case CS_GETINFO:
		request = &mInfoRequest;
		dhdebug("Send info request...");
		break;
	case CS_REGISTER:
		request = &mRegisterRequest;
		dhdebug("Send register request...");
		break;
	case CS_POLL:
	case CS_CUSTOM:
		request = custom_firmware_request();
		if(request) {
			mConnectionState = CS_CUSTOM;
		} else {
			request = &mPollRequest;
			dhdebug("Send poll request...");
		}
		break;
	default:
		dhdebug("ASSERT: networkConnectCb wrong state %d", mConnectionState);
	}
	int res;
	if( (res = espconn_send(&mDHConnector, request->data, request->len)) != ESPCONN_OK) {
		mConnectionState = CS_DISCONNECT;
		dhesperrors_espconn_result("network_connect_cb failed:", res);
		espconn_disconnect(&mDHConnector);
	} else {
		dhstatistic_add_bytes_sent(request->len);
	}
}

LOCAL void network_disconnect_cb(void *arg) {
	switch(mConnectionState) {
	case CS_DISCONNECT:
		arm_repeat_timer(RETRY_CONNECTION_INTERVAL_MS);
		break;
	case CS_GETINFO:
		set_state(CS_REGISTER);
		break;
	case CS_REGISTER:
		mConnectionState = CS_POLL;
		/* no break */
	case CS_POLL:
		arm_repeat_timer(DHREQUEST_PAUSE_MS);
		break;
	case CS_CUSTOM:
		if (dhterminal_is_in_use()) {
			dhdebug("Terminal is in use, no deep sleep");
			arm_repeat_timer(CUSTOM_NOTIFICATION_INTERVAL_MS);
		} else {
			system_deep_sleep(CUSTOM_NOTIFICATION_INTERVAL_MS * 1000);
			// after deep sleep chip will be rebooted
		}
		break;
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
	if (ip == NULL) {
		dhdebug("Resolve %s failed. Trying again...", name);
		mConnectionState = CS_DISCONNECT;
		arm_repeat_timer(RETRY_CONNECTION_INTERVAL_MS);
		dhstatistic_inc_network_errors_count();
		return;
	}
	unsigned char *bip = (unsigned char *) ip;
	dhdebug("Host %s ip: %d.%d.%d.%d, using port %d", name, bip[0], bip[1], bip[2], bip[3], mDHConnector.proto.tcp->remote_port);

	dhsender_init(ip, mDHConnector.proto.tcp->remote_port);
	dhconnector_init_connection(ip);
	if(mPollRequest.len)
		set_state(CS_POLL);
	else
		set_state(CS_GETINFO);
}

LOCAL void ICACHE_FLASH_ATTR start_resolve_dh_server() {
	static ip_addr_t ip;
	const char *server = dhrequest_current_server();
	char host[os_strlen(server) + 1];
	const char *fr = server;
	while(*fr != ':') {
		fr++;
		if(*fr == 0) {
			fr = 0;
			break;
		}
	}
	if(fr) {
		fr++;
		if(*fr != '/')
			fr = 0;
	}
	if (fr) {
		while (*fr == '/')
			fr++;
		int i = 0;
		while (*fr != '/' && *fr != ':' && *fr != 0)
			host[i++] = *fr++;
		// read port if present
		int port = 0;
		if(*fr == ':') {
			unsigned char d;
			fr++;
			while ( (d = *fr - 0x30) < 10) {
				fr++;
				port = port*10 + d;
				if(port > 0xFFFF)
					break;
			}
		}
		if(port && port < 0xFFFF)
			mDHConnector.proto.tcp->remote_port = port;
		else if (os_strncmp(dhrequest_current_server(), "https", 5) == 0)
			mDHConnector.proto.tcp->remote_port = 443; // HTTPS default port
		else
			mDHConnector.proto.tcp->remote_port = 80; //HTTP default port
		host[i] = 0;
		dhdebug("Resolving %s", host);
		err_t r = espconn_gethostbyname(&mDHConnector, host, &ip, resolve_cb);
		if(r == ESPCONN_OK) {
			resolve_cb(host, &ip, NULL);
		} else if(r != ESPCONN_INPROGRESS) {
			dhesperrors_espconn_result("Resolving failed:", r);
			arm_repeat_timer(RETRY_CONNECTION_INTERVAL_MS);
		}
	} else {
		dhdebug("Can not find scheme in server url");
	}
}

void ICACHE_FLASH_ATTR dhmem_unblock_cb() {
	if(mNeedRecover) {
		set_state(mConnectionState);
		mNeedRecover = 0;
	}
}

LOCAL void set_state(CONNECTION_STATE state) {
	mConnectionState = state;
	if(state != CS_DISCONNECT) {
		if(dhmem_isblock()) {
			mNeedRecover = 1;
			return;
		}
	}
	switch(state) {
	case CS_DISCONNECT:
		start_resolve_dh_server();
		break;
	case CS_GETINFO:
	case CS_REGISTER:
	case CS_POLL:
	case CS_CUSTOM:
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
		} else {
			dhdebug("ERROR: WiFi reports STAMODE_GOT_IP, but no actual ip found");
		}
	} else if(event->event == EVENT_STAMODE_DISCONNECTED) {
		os_timer_disarm(&mRetryTimer);
		dhsender_stop_repeat();
		dhesperrors_disconnect_reason("WiFi disconnected", event->event_info.disconnected.reason);
		dhstatistic_inc_wifi_lost_count();
	} else {
		dhesperrors_wifi_state("WiFi event", event->event);
	}
}

void ICACHE_FLASH_ATTR dhconnector_init(dhconnector_command_json_cb cb) {
	dhrequest_load_settings();
	mCommandCallback = cb;
	mConnectionState = CS_DISCONNECT;

	dhrequest_create_info(&mInfoRequest);
	dhrequest_create_register(&mRegisterRequest);
	mPollRequest.len = mPollRequest.data[0] = 0;

	wifi_set_opmode(STATION_MODE);
	wifi_station_set_auto_connect(1);
	wifi_station_set_reconnect_policy(true);
	struct station_config stationConfig;
	wifi_station_get_config(&stationConfig);
	wifi_set_phy_mode(PHY_MODE_11N);
	os_memset(stationConfig.ssid, 0, sizeof(stationConfig.ssid));
	os_memset(stationConfig.password, 0, sizeof(stationConfig.password));
	snprintf(stationConfig.ssid, sizeof(stationConfig.ssid), "%s", dhsettings_get_wifi_ssid());
	snprintf(stationConfig.password, sizeof(stationConfig.password), "%s", dhsettings_get_wifi_password());
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

CONNECTION_STATE ICACHE_FLASH_ATTR dhconnector_get_state() {
	return mConnectionState;
}
