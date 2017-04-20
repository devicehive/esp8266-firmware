/*
 * dhterminal_command.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Commandline command implementation
 *
 */
#include "dhterminal_commands.h"
#include "dhterminal.h"
#include "DH/uart.h"
#include "snprintf.h"
#include "dhconnector.h"
#include "dhsettings.h"
#include "dhterminal_configure.h"
#include "dhutils.h"
#include "user_config.h"
#include "dhstatistic.h"
#include "dhsender_queue.h"

#include <ets_sys.h>
#include <osapi.h>
#include <user_interface.h>
#include <espconn.h>
#include <ping.h>
#include <ets_forward.h>

int mIsCommandWorking = 0;
char mHostBuffer[80];

int ICACHE_FLASH_ATTR dhterminal_commands_is_busy(void) {
	return mIsCommandWorking;
}

LOCAL void ICACHE_FLASH_ATTR sprintIp(char *buff, const struct ip_addr *ip) {
	unsigned char *bip = (unsigned char *)ip;
	snprintf(buff, 16, "%d.%d.%d.%d", bip[0], bip[1], bip[2], bip[3]);
}

LOCAL void ICACHE_FLASH_ATTR printBytes(char *buff, unsigned long long bytes) {
	if(bytes > 1099511627776LL)
		snprintf(buff, 16, "%f TiB", bytes/1099511627776.0);
	else if(bytes > 1073741824)
		snprintf(buff, 16, "%f GiB", bytes/1073741824.0);
	else if(bytes > 1048576)
		snprintf(buff, 16, "%f MiB", bytes/1048576.0);
	else if(bytes > 1024)
		snprintf(buff, 16, "%f KiB", bytes/1024.0);
	else
		snprintf(buff, 16, "%u B", (unsigned int)bytes);
}

LOCAL void ICACHE_FLASH_ATTR sprintMac(char *buff, const uint8 *mac) {
	int i;
	for(i = 0; i < 6; i++) {
		if(i)
			*buff++ = ':';
		buff += snprintf(buff, 3, mac[i] < 0x10 ? "0%X" : "%X", mac[i]);
	}
}

void ICACHE_FLASH_ATTR dhterminal_commands_uname(const char *args) {
	char digitBuff[32];
	dh_uart_send_line("DeviceHive ESP8266 firmware v"FIRMWARE_VERSION" [Built: "__TIME__" "__DATE__"]");
	dh_uart_send_line("Git revision: "FIRMWARE_GIT_REVISION);
	dh_uart_send_line("Created by Nikolay Khabarov.");
	dh_uart_send_str("ChipID: 0x");
	snprintf(digitBuff, sizeof(digitBuff), "%X", system_get_chip_id());
	dh_uart_send_str(digitBuff);
	dh_uart_send_str(", SDK version: ");
	dh_uart_send_str(system_get_sdk_version());
	dh_uart_send_line("");
}

LOCAL void ICACHE_FLASH_ATTR printIpInfo(uint8 if_index) {
	struct ip_info info;
	char digitBuff[32];
	if(!wifi_get_ip_info(if_index, &info)) {
		dh_uart_send_line("Failed to get ip info");
	} else {
		dh_uart_send_str("IP: ");
		sprintIp(digitBuff, &info.ip);
		dh_uart_send_str(digitBuff);
		dh_uart_send_str(", netmask: ");
		sprintIp(digitBuff, &info.netmask);
		dh_uart_send_str(digitBuff);
		dh_uart_send_str(", gateway: ");
		sprintIp(digitBuff, &info.gw);
		dh_uart_send_line(digitBuff);
	}
}

void ICACHE_FLASH_ATTR dhterminal_commands_status(const char *args) {
	uint8 mac[6];
	char digitBuff[32];

	const struct DHStat *stat = dhstat_get();

	dh_uart_send_str("Network adapter ");
	if(!wifi_get_macaddr(STATION_IF, mac)) {
		dh_uart_send_str("[Failed to get mac address]");
	} else {
		sprintMac(digitBuff, mac);
		dh_uart_send_str(digitBuff);
	}

	if(wifi_get_opmode() == SOFTAP_MODE) {
		struct softap_config softapConfig;
		dh_uart_send_str(" is in AP mode, SSID ");
		wifi_softap_get_config(&softapConfig);
		dh_uart_send_line((const char*)softapConfig.ssid);
		printIpInfo(SOFTAP_IF);
	} else {
		struct station_config stationConfig;
		if(!wifi_station_get_config(&stationConfig)) {
			os_memset(&stationConfig, 0, sizeof(stationConfig));
			os_strcpy(stationConfig.ssid, "[Can not get SSID]");
		}
		const char *ssid = (const char*)stationConfig.ssid;
		switch(wifi_station_get_connect_status()) {
		case STATION_IDLE:
			dh_uart_send_line(" is in idle");
			break;
		case STATION_CONNECTING:
			dh_uart_send_str(" is connecting to ");
			dh_uart_send_line(ssid);
			break;
		case STATION_WRONG_PASSWORD:
			dh_uart_send_str(" has wrong password for ");
			dh_uart_send_line(ssid);
			break;
		case STATION_NO_AP_FOUND:
			dh_uart_send_str(" can not find AP with SSID ");
			dh_uart_send_line(ssid);
			break;
		case STATION_CONNECT_FAIL:
			dh_uart_send_str(" has fail while connecting to ");
			dh_uart_send_line(ssid);
			break;
		case STATION_GOT_IP:
		{
			dh_uart_send_str(" is connected to ");
			dh_uart_send_line(ssid);
			printIpInfo(STATION_IF);
			break;
		}
		default:
			dh_uart_send_line(" is in unknown state");
			break;
		}
		dh_uart_send_str("Wi-Fi disconnect count: ");
		snprintf(digitBuff, sizeof(digitBuff), "%u", stat->wifiLosts);
		dh_uart_send_line(digitBuff);
	}

	dh_uart_send_str("Bytes received: ");
	printBytes(digitBuff, stat->bytesReceived);
	dh_uart_send_str(digitBuff);
	dh_uart_send_str(", sent: ");
	printBytes(digitBuff, stat->bytesSent);
	dh_uart_send_str(digitBuff);
	dh_uart_send_str(", errors: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u", stat->networkErrors);
	dh_uart_send_line(digitBuff);

	dh_uart_send_str("Httpd requests received: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u", stat->httpdRequestsCount);
	dh_uart_send_str(digitBuff);
	dh_uart_send_str(", errors: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u", stat->httpdErrorsCount);
	dh_uart_send_line(digitBuff);

	dh_uart_send_str("DeviceHive: ");
	switch(dhconnector_get_state()) {
	case CS_DISCONNECT:
		dh_uart_send_str("connection is not established");
		break;
	case CS_GETINFO:
		dh_uart_send_str("getting info from server");
		break;
	case CS_RESOLVEWEBSOCKET:
	case CS_WEBSOCKET:
		dh_uart_send_str("connecting to web socket");
		break;
	case CS_OPERATE:
		dh_uart_send_str("successfully connected to server");
		break;
	default:
		dh_uart_send_str("unknown state");
		break;
	}
	dh_uart_send_str(", errors count: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u", stat->serverErrors);
	dh_uart_send_line(digitBuff);

	dh_uart_send_str("Responses created/dropped: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u/%u", stat->responcesTotal, stat->responcesDroppedCount);
	dh_uart_send_str(digitBuff);
	dh_uart_send_str(", notification created/dropped: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u/%u", stat->notificationsTotal, stat->notificationsDroppedCount);
	dh_uart_send_line(digitBuff);

	dh_uart_send_str("Local REST requests/errors: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u/%u", stat->localRestRequestsCount, stat->localRestResponcesErrors);
	dh_uart_send_line(digitBuff);

	dh_uart_send_str("Free heap size: ");
	snprintf(digitBuff, sizeof(digitBuff), "%d", system_get_free_heap_size());
	dh_uart_send_str(digitBuff);
	dh_uart_send_str(" bytes");

	dh_uart_send_str(", request queue size: ");
	snprintf(digitBuff, sizeof(digitBuff), "%d", dhsender_queue_length());
	dh_uart_send_line(digitBuff);

}

LOCAL void ICACHE_FLASH_ATTR scan_done_cb (void *arg, STATUS status) {
	char buff[80];
	int i;
	if(dhterminal_get_mode() == SM_OUTPUT_MODE) {
		if(status != OK) {
			dh_uart_send_line("scan failed");
			return;
		}
		struct bss_info *link = (struct bss_info *)arg;
		link = link->next.stqe_next; //ignore first according to the sdk manual
		if(link)
			dh_uart_send_line("SSID                             Rssi      Channel  Auth      BSSID");
		else
			dh_uart_send_line("No wireless networks found.");
		while(link) {
			char * auth = 0;
			switch(link->authmode) {
			case AUTH_OPEN:
				auth = "open";
				break;
		    case AUTH_WEP:
		    	auth = "WEB";
		    	break;
		    case AUTH_WPA_PSK:
		    	auth = "WPA";
		    	break;
		    case AUTH_WPA2_PSK:
		    	auth = "WPA2";
		    	break;
		    case AUTH_WPA_WPA2_PSK:
		    	auth = "WPA/WPA2";
		    	break;
		    case AUTH_MAX:
		    	auth = "MAX";
		    	break;
			}
			os_memset(&buff, 0, sizeof(buff));
			snprintf(buff, 32, "%s", link->is_hidden ? "<HIDDEN NETWORK>" : (char *)link->ssid);
			snprintf(&buff[33], 9,  "%d dBm", link->rssi);
			snprintf(&buff[43], 8,  "%d", link->channel);
			snprintf(&buff[52], 10,  "%s", auth);
			sprintMac(&buff[62], link->bssid);
			buff[sizeof(buff) - 1] = 0;
			for(i=0; i < sizeof(buff) - 1; i++) {
				if(buff[i] == 0)
					buff[i] = ' ';
			}
			dh_uart_send_line(buff);
			link = link->next.stqe_next;
		}
		dhterminal_set_mode(SM_NORMAL_MODE, 0, 0, 0, 0);
	}
	mIsCommandWorking = 0;
}

void ICACHE_FLASH_ATTR dhterminal_commands_scan(const char *args) {
	static struct scan_config config = {0, 0, 0, 1};
	if(wifi_station_scan(&config, scan_done_cb)) {
		dh_uart_send_line("Scanning...");
		mIsCommandWorking = 1;
		dhterminal_set_mode(SM_OUTPUT_MODE, 0, 0, 0, 0);
	} else {
		dh_uart_send_line("Failed to start scan.");
	}
}

void ICACHE_FLASH_ATTR dhterminal_commands_debug(const char *args) {
	dh_uart_send_line("Debug output enabled. Press 'q' for exit.");
	dhterminal_set_mode(SM_DEBUG_MODE, 0, 0, 0, 0);
	dh_uart_send_str(dhterminal_get_debug_ouput());
}

void ICACHE_FLASH_ATTR dhterminal_commands_history(const char *args) {
	const char *tmp = dhterminal_get_history();
	while(*tmp) {
		dh_uart_send_line(tmp);
		while(*tmp++);
	}
}

void ICACHE_FLASH_ATTR dhterminal_commands_reboot(const char *args) {
	dh_uart_send_line("Rebooting...");
	system_restart();
}

void ICACHE_FLASH_ATTR dhterminal_commands_config(const char *args) {
	dh_uart_send_str("Wi-Fi Mode: ");
	switch(dhsettings_get_wifi_mode()) {
	case WIFI_MODE_CLIENT:
		dh_uart_send_line("Client");
		break;
	case WIFI_MODE_AP:
		dh_uart_send_line("Access Point");
		break;
	}
	dh_uart_send_str("Wi-Fi SSID: ");
	dh_uart_send_line(dhsettings_get_wifi_ssid());
	dh_uart_send_str("DeviceHive Server: ");
	dh_uart_send_line(dhsettings_get_devicehive_server());
	dh_uart_send_str("DeviceHive DeviceId: ");
	dh_uart_send_line(dhsettings_get_devicehive_deviceid());
}

void ICACHE_FLASH_ATTR dhterminal_commands_configure(const char *args) {
	int force = (os_strcmp(args, "--force-clear") == 0) ? 1 : 0;
	if(force || os_strcmp(args, "--clear") == 0) {
		if(dhsettings_clear(force)) {
			dh_uart_send_line("Settings was cleared, rebooting...");
			system_restart();
		} else {
			dh_uart_send_line("Error while cleaning settings.");
		}
	} else {
		dhterminal_configure_start();
	}
}

void ICACHE_FLASH_ATTR dhterminal_commands_echo(const char *args) {
	dh_uart_send_line(args);
}

LOCAL void ICACHE_FLASH_ATTR nslookup_res(const char *name, ip_addr_t *ip, void *arg) {
	char ipstr[16];
	if(ip == NULL) {
		dh_uart_send_line("FAILED");
	} else {
		sprintIp(ipstr, ip);
		dh_uart_send_str("Host ");
		dh_uart_send_str(name);
		dh_uart_send_str(" IP is ");
		dh_uart_send_line(ipstr);
	}
}

LOCAL void ICACHE_FLASH_ATTR nslookup_cb(const char *name, ip_addr_t *ip, void *arg) {
	if(dhterminal_get_mode() == SM_OUTPUT_MODE) {
		nslookup_res(name, ip, arg);
		dhterminal_set_mode(SM_NORMAL_MODE, 0, 0, 0, 0);
	}
	mIsCommandWorking = 0;
}

LOCAL void ICACHE_FLASH_ATTR startResolve(const char *domain, dns_found_callback output_mode_cb, dns_found_callback res_cb) {
	static ip_addr_t ip;
	static struct espconn connector = {0};
	err_t r = espconn_gethostbyname(&connector, domain, &ip, output_mode_cb);
	dh_uart_send_line("Resolving...");
	if(r == ESPCONN_OK) {
		res_cb(domain, &ip, &connector);
	} else if(r != ESPCONN_INPROGRESS) {
		dh_uart_send_line("ERROR: illegal request");
	} else {
		mIsCommandWorking = 1;
		dhterminal_set_mode(SM_OUTPUT_MODE, 0, 0, 0, 0);
	}
}

void ICACHE_FLASH_ATTR dhterminal_commands_nslookup(const char *args) {
	snprintf(mHostBuffer, sizeof(mHostBuffer), args);
	startResolve(mHostBuffer, nslookup_cb, nslookup_res);
}

// workarounds for ping features in SDK v1.1.2
int mSent, mRecieved, mLost, mTotalDelay;
struct ping_option mCurrentPinopt = {0};
LOCAL void ICACHE_FLASH_ATTR ping_cb(void* arg, void *pdata) {
	char digitbuf[16];
	struct ping_option *pingopt = (struct ping_option *)arg;
	struct ping_resp *pingresp = (struct ping_resp *)pdata;
	if(dhterminal_get_mode() == SM_OUTPUT_MODE) {
		mSent++;
		if(pingresp->ping_err == 0) {
			mRecieved++;
			mTotalDelay += pingresp->resp_time;
			snprintf(digitbuf, sizeof(digitbuf), "%d", pingresp->bytes);
			dh_uart_send_str(digitbuf);
			dh_uart_send_str(" bytes received from ");
			sprintIp(digitbuf, (const struct ip_addr *)&pingopt->ip);
			dh_uart_send_str(digitbuf);
			dh_uart_send_str(" in ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", pingresp->resp_time);
			dh_uart_send_str(digitbuf);
			dh_uart_send_str(" ms, icmp_seq = ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", mSent);
			dh_uart_send_line(digitbuf);
		} else {
			mLost++;
			dh_uart_send_str("Request timed out, icmp_seq = ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", mSent);
			dh_uart_send_line(digitbuf);
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR ping_done_cb(void* arg, void *pdata) {
	char digitbuf[16];
	if(dhterminal_get_mode() == SM_OUTPUT_MODE) {
		dh_uart_send_str("Total sent: ");
		snprintf(digitbuf, sizeof(digitbuf), "%d", mSent);
		dh_uart_send_str(digitbuf);
		dh_uart_send_str(", received: ");
		snprintf(digitbuf, sizeof(digitbuf), "%d", mRecieved);
		dh_uart_send_str(digitbuf);
		dh_uart_send_str(", lost: ");
		snprintf(digitbuf, sizeof(digitbuf), "%d", mLost);
		dh_uart_send_str(digitbuf);
		if(mRecieved) {
			dh_uart_send_str(", average time: ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", mTotalDelay/mRecieved);
			dh_uart_send_str(digitbuf);
			dh_uart_send_line(" ms");
		} else {
			dh_uart_send_line("");
		}
		dhterminal_set_mode(SM_NORMAL_MODE, 0, 0, 0, 0);
	}
	mIsCommandWorking = 0;
}

LOCAL void ICACHE_FLASH_ATTR ping_res_cb(const char *name, ip_addr_t *ip, void *arg) {
	nslookup_res(name, ip, arg);
	if(ip) {
		mCurrentPinopt.ip = ip->addr;
		mCurrentPinopt.count = 4;
		mCurrentPinopt.coarse_time = 1;
		mCurrentPinopt.recv_function = ping_cb;
		mCurrentPinopt.sent_function = ping_done_cb;
		mSent = 0;
		mRecieved = 0;
		mLost = 0;
		mTotalDelay  = 0;
		ping_start(&mCurrentPinopt);
	}
}

LOCAL void ICACHE_FLASH_ATTR ping_nslookup_cb(const char *name, ip_addr_t *ip, void *arg) {
	if(dhterminal_get_mode() == SM_OUTPUT_MODE) {
		ping_res_cb(name, ip, arg);
		if(ip == 0) {
			mIsCommandWorking = 0;
			dhterminal_set_mode(SM_NORMAL_MODE, 0, 0, 0, 0);
		}
	} else {
		mIsCommandWorking = 0;
	}
}

LOCAL void ICACHE_FLASH_ATTR ping_im_cb(const char *name, ip_addr_t *ip, void *arg) {
	if(ip) {
		mIsCommandWorking = 1;
		dhterminal_set_mode(SM_OUTPUT_MODE, 0, 0, 0, 0);
	}
	ping_res_cb(name, ip, arg);
}

void ICACHE_FLASH_ATTR dhterminal_commands_ping(const char *args) {
	snprintf(mHostBuffer, sizeof(mHostBuffer), args);
	startResolve(mHostBuffer, ping_nslookup_cb, ping_im_cb);
}
