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

#include <ets_sys.h>
#include <osapi.h>
#include <user_interface.h>
#include <espconn.h>
#include <ping.h>
#include "dhterminal.h"
#include "dhuart.h"
#include "snprintf.h"
#include "dhconnector.h"
#include "dhterminal_commands.h"
#include "dhsettings.h"
#include "dhterminal_configure.h"
#include "dhutils.h"
#include "user_config.h"
#include "dhstatistic.h"
#include "dhsender_queue.h"

int mIsCommandWorking = 0;
char mHostBuffer[80];

int ICACHE_FLASH_ATTR dhterminal_commands_is_busy() {
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
	else if (bytes > 1048576)
		snprintf(buff, 16, "%f MiB", bytes/1048576.0);
	else if (bytes > 1024)
		snprintf(buff, 16, "%f KiB", bytes/1024.0);
	else
		snprintf(buff, 16, "%u B", (unsigned long)bytes);
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
	dhuart_send_line("DeviceHive ESP8266 firmware v"FIRMWARE_VERSION" [Built: "__TIME__" "__DATE__"]");
	dhuart_send_line("Created by Nikolay Khabarov.");
	dhuart_send_str("ChipID: 0x");
	snprintf(digitBuff, sizeof(digitBuff), "%X", system_get_chip_id());
	dhuart_send_str(digitBuff);
	dhuart_send_str(", SDK version: ");
	dhuart_send_str(system_get_sdk_version());
	dhuart_send_line("");
}

void ICACHE_FLASH_ATTR dhterminal_commands_status(const char *args) {
	uint8 mac[6];
	char digitBuff[32];
	struct station_config stationConfig;

	dhuart_send_str("Network adapter ");
	if(!wifi_get_macaddr(STATION_IF, mac)) {
		dhuart_send_str("[Failed to get mac address]");
	} else {
		sprintMac(digitBuff, mac);
		dhuart_send_str(digitBuff);
	}

	if(!wifi_station_get_config(&stationConfig)) {
		os_memset(&stationConfig, 0, sizeof(stationConfig));
		os_strcpy(stationConfig.ssid, "[Can not get SSID]");
	}
	switch(wifi_station_get_connect_status()) {
	case STATION_IDLE:
		dhuart_send_line(" is in idle");
		break;
	case STATION_CONNECTING:
		dhuart_send_str(" is connecting to ");
		dhuart_send_line(stationConfig.ssid);
		break;
	case STATION_WRONG_PASSWORD:
		dhuart_send_str(" has wrong password for ");
		dhuart_send_line(stationConfig.ssid);
		break;
	case STATION_NO_AP_FOUND:
		dhuart_send_str(" can not find AP with SSID ");
		dhuart_send_line(stationConfig.ssid);
		break;
	case STATION_CONNECT_FAIL:
		dhuart_send_str(" has fail while connecting to ");
		dhuart_send_line(stationConfig.ssid);
		break;
	case STATION_GOT_IP:
	{
		dhuart_send_str(" is connected to ");
		dhuart_send_line(stationConfig.ssid);
		struct ip_info info;
		if(!wifi_get_ip_info(STATION_IF, &info)) {
			dhuart_send_line("Failed to get ip info");
		} else {
			dhuart_send_str("IP: ");
			sprintIp(digitBuff, &info.ip);
			dhuart_send_str(digitBuff);
			dhuart_send_str(", netmask: ");
			sprintIp(digitBuff, &info.netmask);
			dhuart_send_str(digitBuff);
			dhuart_send_str(", gateway: ");
			sprintIp(digitBuff, &info.gw);
			dhuart_send_line(digitBuff);
		}
		break;
	}
	default:
		dhuart_send_line("is in unknown state");
		break;
	}
	const DHSTATISTIC *stat = dhstatistic_get_statistic();
	dhuart_send_str("Wi-Fi disconnect count: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u", stat->wifiLosts);
	dhuart_send_line(digitBuff);

	dhuart_send_str("Bytes received: ");
	printBytes(digitBuff, stat->bytesReceived);
	dhuart_send_str(digitBuff);
	dhuart_send_str(", sent: ");
	printBytes(digitBuff, stat->bytesSent);
	dhuart_send_str(digitBuff);
	dhuart_send_str(", errors: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u", stat->networkErrors);
	dhuart_send_line(digitBuff);

	dhuart_send_str("Httpd requests received: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u", stat->httpdRequestsCount);
	dhuart_send_str(digitBuff);
	dhuart_send_str(", errors: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u", stat->httpdErrorsCount);
	dhuart_send_line(digitBuff);

	dhuart_send_str("DeviceHive: ");
	switch(dhconnector_get_state()) {
	case CS_DISCONNECT:
		dhuart_send_str("connection is not established");
		break;
	case CS_GETINFO:
		dhuart_send_str("getting info from server");
		break;
	case CS_REGISTER:
		dhuart_send_str("registering device");
		break;
	case CS_POLL:
	case CS_CUSTOM:
		dhuart_send_str("successfully connected to server");
		break;
	default:
		dhuart_send_str("unknown state");
		break;
	}
	dhuart_send_str(", errors count: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u", stat->serverErrors);
	dhuart_send_line(digitBuff);

	dhuart_send_str("Responses created/dropped: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u/%u", stat->responcesTotal, stat->responcesDroppedCount);
	dhuart_send_str(digitBuff);
	dhuart_send_str(", notification created/dropped: ");
	snprintf(digitBuff, sizeof(digitBuff), "%u/%u", stat->notificationsTotal, stat->notificationsDroppedCount);
	dhuart_send_line(digitBuff);


	dhuart_send_str("Free heap size: ");
	snprintf(digitBuff, sizeof(digitBuff), "%d", system_get_free_heap_size());
	dhuart_send_str(digitBuff);
	dhuart_send_str(" bytes");

	dhuart_send_str(", request queue size: ");
	snprintf(digitBuff, sizeof(digitBuff), "%d", dhsender_queue_length());
	dhuart_send_line(digitBuff);

}

LOCAL void ICACHE_FLASH_ATTR scan_done_cb (void *arg, STATUS status) {
	char buff[80];
	int i;
	if(dhterminal_get_mode() == SM_OUTPUT_MODE) {
		if(status != OK) {
			dhuart_send_line("scan failed");
			return;
		}
		struct bss_info *link = (struct bss_info *)arg;
		link = link->next.stqe_next; //ignore first according to the sdk manual
		if(link)
			dhuart_send_line("SSID                             Rssi      Channel  Auth      BSSID");
		else
			dhuart_send_line("No wireless networks found.");
		while(link) {
			char * auth;
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
			dhuart_send_line(buff);
			link = link->next.stqe_next;
		}
		dhterminal_set_mode(SM_NORMAL_MODE, 0, 0, 0, 0);
	}
	mIsCommandWorking = 0;
}

void ICACHE_FLASH_ATTR dhterminal_commands_scan(const char *args) {
	static struct scan_config config = {0, 0, 0, 1};
	if(wifi_station_scan(&config, scan_done_cb)) {
		dhuart_send_line("Scanning...");
		mIsCommandWorking = 1;
		dhterminal_set_mode(SM_OUTPUT_MODE, 0, 0, 0, 0);
	} else {
		dhuart_send_line("Failed to start scan.");
	}
}

void ICACHE_FLASH_ATTR dhterminal_commands_debug(const char *args) {
	dhuart_send_line("Debug output enabled. Press 'q' for exit.");
	dhterminal_set_mode(SM_DEBUG_MODE, 0, 0, 0, 0);
	dhuart_send_str(dhterminal_get_debug_ouput());
}

void ICACHE_FLASH_ATTR dhterminal_commands_history(const char *args) {
	const char *tmp = dhterminal_get_history();
	while(*tmp) {
		dhuart_send_line(tmp);
		while(*tmp++);
	}
}

void ICACHE_FLASH_ATTR dhterminal_commands_reboot(const char *args) {
	dhuart_send_line("Rebooting...");
	system_restart();
}

void ICACHE_FLASH_ATTR dhterminal_commands_config(const char *args) {
	dhuart_send_str("Wi-Fi SSID: ");
	dhuart_send_line(dhsettings_get_wifi_ssid());
	dhuart_send_str("DeviceHive Server: ");
	dhuart_send_line(dhsettings_get_devicehive_server());
	dhuart_send_str("DeviceHive DeviceId: ");
	dhuart_send_line(dhsettings_get_devicehive_deviceid());
}

void ICACHE_FLASH_ATTR dhterminal_commands_configure(const char *args) {
	int force = (os_strcmp(args, "--force-clear") == 0) ? 1 : 0;
	if(force || os_strcmp(args, "--clear") == 0) {
		if(dhsettings_clear(force)) {
			dhuart_send_line("Settings was cleared, rebooting...");
			system_restart();
		} else {
			dhuart_send_line("Error while cleaning settings.");
		}
	} else {
		dhterminal_configure_start();
	}
}

void ICACHE_FLASH_ATTR dhterminal_commands_echo(const char *args) {
	dhuart_send_line(args);
}

LOCAL void ICACHE_FLASH_ATTR nslookup_res(const char *name, ip_addr_t *ip, void *arg) {
	char ipstr[16];
	if (ip == NULL) {
		dhuart_send_line("FAILED");
	} else {
		sprintIp(ipstr, ip);
		dhuart_send_str("Host ");
		dhuart_send_str(name);
		dhuart_send_str(" IP is ");
		dhuart_send_line(ipstr);
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
	dhuart_send_line("Resolving...");
	if(r == ESPCONN_OK) {
		res_cb(domain, &ip, &connector);
	} else if(r != ESPCONN_INPROGRESS) {
		dhuart_send_line("ERROR: illegal request");
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
			dhuart_send_str(digitbuf);
			dhuart_send_str(" bytes received from ");
			sprintIp(digitbuf, (const struct ip_addr *)&pingopt->ip);
			dhuart_send_str(digitbuf);
			dhuart_send_str(" in ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", pingresp->resp_time);
			dhuart_send_str(digitbuf);
			dhuart_send_str(" ms, icmp_seq = ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", mSent);
			dhuart_send_line(digitbuf);
		} else {
			mLost++;
			dhuart_send_str("Request timed out, icmp_seq = ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", mSent);
			dhuart_send_line(digitbuf);
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR ping_done_cb(void* arg, void *pdata) {
	char digitbuf[16];
	if(dhterminal_get_mode() == SM_OUTPUT_MODE) {
		dhuart_send_str("Total sent: ");
		snprintf(digitbuf, sizeof(digitbuf), "%d", mSent);
		dhuart_send_str(digitbuf);
		dhuart_send_str(", received: ");
		snprintf(digitbuf, sizeof(digitbuf), "%d", mRecieved);
		dhuart_send_str(digitbuf);
		dhuart_send_str(", lost: ");
		snprintf(digitbuf, sizeof(digitbuf), "%d", mLost);
		dhuart_send_str(digitbuf);
		if(mRecieved) {
			dhuart_send_str(", average time: ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", mTotalDelay/mRecieved);
			dhuart_send_str(digitbuf);
			dhuart_send_line(" ms");
		} else {
			dhuart_send_line("");
		}
		dhterminal_set_mode(SM_NORMAL_MODE, 0, 0, 0, 0);
	}
	mIsCommandWorking = 0;
}

LOCAL void ICACHE_FLASH_ATTR ping_res_cb(const char *name, ip_addr_t *ip, void *arg) {
	nslookup_res(name, ip, arg);
	if (ip) {
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
		if (ip == 0) {
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
