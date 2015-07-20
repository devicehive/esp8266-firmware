/*
 * dhserial_command.c
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
#include "dhserial.h"
#include "drivers/uart.h"
#include "snprintf.h"
#include "dhconnector.h"
#include "dhserial_commands.h"
#include "dhsettings.h"
#include "dhserial_configure.h"
#include "user_config.h"

int mIsCommandWorking = 0;

int ICACHE_FLASH_ATTR dhserial_commands_is_busy() {
	return mIsCommandWorking;
}

LOCAL void ICACHE_FLASH_ATTR sprintIp(char *buff, const struct ip_addr *ip) {
	unsigned char *bip = (unsigned char *)ip;
	snprintf(buff, 16, "%d.%d.%d.%d", bip[0], bip[1], bip[2], bip[3]);
}

LOCAL void ICACHE_FLASH_ATTR sprintMac(char *buff, const uint8 *mac) {
	int i;
	for(i = 0; i < 6; i++) {
		if(i)
			*buff++ = ':';
		buff += snprintf(buff, 3, mac[i] < 0x10 ? "0%X" : "%X", mac[i]);
	}
}

void ICACHE_FLASH_ATTR dhserial_commands_uname(const char *args) {
	char digitBuff[32];
	uart_send_line("DeviceHive ESP8266 firmware "FIRMWARE_VERSION" [Built: "__TIME__" "__DATE__"]");
	uart_send_line("Created by Nikolay Khabarov.");
	uart_send_str("ChipID: 0x");
	snprintf(digitBuff, sizeof(digitBuff), "%X", system_get_chip_id());
	uart_send_str(digitBuff);
	uart_send_str(", SDK version: ");
	uart_send_str(system_get_sdk_version());
	uart_send_line("");
}

void ICACHE_FLASH_ATTR dhserial_commands_status(const char *args) {
	uint8 mac[6];
	char digitBuff[32];
	if(!wifi_get_macaddr(STATION_IF, mac)) {
		uart_send_line("Failed to get mac address");
	} else {
		struct station_config stationConfig;
		system_print_meminfo();
		if(!wifi_station_get_config(&stationConfig)) {
			uart_send_line("Could not get station config");
			os_memset(&stationConfig, 0, sizeof(stationConfig));
		}
		uart_send_str("Network adapter ");
		sprintMac(digitBuff, mac);
		uart_send_str(digitBuff);
		switch(wifi_station_get_connect_status()) {
		case STATION_IDLE:
			uart_send_line(" is in idle");
			break;
		case STATION_CONNECTING:
			uart_send_str(" is connecting to ");
			uart_send_line(stationConfig.ssid);
			break;
		case STATION_WRONG_PASSWORD:
			uart_send_str(" has wrong password for ");
			uart_send_line(stationConfig.ssid);
			break;
		case STATION_NO_AP_FOUND:
			uart_send_str(" can not find AP with SSID ");
			uart_send_line(stationConfig.ssid);
			break;
		case STATION_CONNECT_FAIL:
			uart_send_str(" has fail while connecting to ");
			uart_send_line(stationConfig.ssid);
			break;
		case STATION_GOT_IP:
		{
			uart_send_str(" is connected to ");
			uart_send_line(stationConfig.ssid);
			struct ip_info info;
			if(!wifi_get_ip_info(STATION_IF, &info)) {
				uart_send_line("Failed to get ip info");
			} else {
				uart_send_str("IP: ");
				sprintIp(digitBuff, &info.ip);
				uart_send_str(digitBuff);
				uart_send_str(", netmask: ");
				sprintIp(digitBuff, &info.netmask);
				uart_send_str(digitBuff);
				uart_send_str(", gateway: ");
				sprintIp(digitBuff, &info.gw);
				uart_send_line(digitBuff);
			}
			break;
		}
		default:
			uart_send_line("is in unknown state");
			break;
		}
		uart_send_str("DeviceHive: ");
		switch(dhconnector_get_state()) {
		case CS_DISCONNECT:
			uart_send_line("connection is not established.");
			break;
		case CS_GETINFO:
			uart_send_line("getting info from server.");
			break;
		case CS_REGISTER:
			uart_send_line("registering device.");
			break;
		case CS_POLL:
			uart_send_line("successfully connected to server.");
			break;
		default:
			uart_send_line("unknown state");
			break;
		}
	}
	uart_send_str("Free heap size: ");
	snprintf(digitBuff, sizeof(digitBuff), "%d", system_get_free_heap_size());
	uart_send_str(digitBuff);
	uart_send_line(" bytes.");
}

LOCAL void ICACHE_FLASH_ATTR scan_done_cb (void *arg, STATUS status) {
	char buff[80];
	int i;
	if(dhserial_get_mode() == SM_OUTPUT_MODE) {
		if(status != OK) {
			uart_send_line("scan failed");
			return;
		}
		struct bss_info *link = (struct bss_info *)arg;
		link = link->next.stqe_next; //ignore first according to the sdk manual
		if(link)
			uart_send_line("SSID                             Rssi      Channel  Auth      BSSID");
		else
			uart_send_line("No wireless networks found.");
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
			uart_send_line(buff);
			link = link->next.stqe_next;
		}
		dhserial_set_mode(SM_NORMAL_MODE, 0, 0);
	}
	mIsCommandWorking = 0;
}

void ICACHE_FLASH_ATTR dhserial_commands_scan(const char *args) {
	static struct scan_config config = {0, 0, 0, 1};
	if(wifi_station_scan(&config, scan_done_cb)) {
		uart_send_line("Scanning...");
		mIsCommandWorking = 1;
		dhserial_set_mode(SM_OUTPUT_MODE, 0, 0);
	} else {
		uart_send_line("Failed to start scan.");
	}
}

void ICACHE_FLASH_ATTR dhserial_commands_debug(const char *args) {
	uart_send_line("Debug output enabled. Press 'q' for exit.");
	dhserial_set_mode(SM_DEBUG_MODE, 0, 0);
	uart_send_str(dhserial_get_debug_ouput());
}

void ICACHE_FLASH_ATTR dhserial_commands_history(const char *args) {
	const char *tmp = dhserial_get_history();
	while(*tmp) {
		uart_send_line(tmp);
		while(*tmp++);
	}
}

void ICACHE_FLASH_ATTR dhserial_commands_reboot(const char *args) {
	uart_send_line("Rebooting...");
	system_restart();
}

void ICACHE_FLASH_ATTR dhserial_commands_config(const char *args) {
	uart_send_str("Wi-Fi SSID: ");
	uart_send_line(dhsettings_get_wifi_ssid());
	uart_send_str("DeviceHive Server: ");
	uart_send_line(dhsettings_get_devicehive_server());
	uart_send_str("DeviceHive DeviceId: ");
	uart_send_line(dhsettings_get_devicehive_deviceid());
}

void ICACHE_FLASH_ATTR dhserial_commands_configure(const char *args) {
	if(os_strcmp(args, "--clear") == 0) {
		if(dhsettings_clear()) {
			uart_send_line("Settings was cleared, rebooting...");
			system_restart();
		} else {
			uart_send_line("Error while cleaning settings");
		}
	} else {
		dhserial_configure_start();
	}
}

void ICACHE_FLASH_ATTR dhserial_commands_echo(const char *args) {
	uart_send_line(args);
}

LOCAL void ICACHE_FLASH_ATTR nslookup_res(const char *name, ip_addr_t *ip, void *arg) {
	char ipstr[16];
	if (ip == NULL) {
		uart_send_line("FAILED");
	} else {
		sprintIp(ipstr, ip);
		uart_send_str("Host ");
		uart_send_str(name);
		uart_send_str(" IP is ");
		uart_send_line(ipstr);
	}
}

LOCAL void ICACHE_FLASH_ATTR nslookup_cb(const char *name, ip_addr_t *ip, void *arg) {
	if(dhserial_get_mode() == SM_OUTPUT_MODE) {
		nslookup_res(name, ip, arg);
		dhserial_set_mode(SM_NORMAL_MODE, 0, 0);
	}
	mIsCommandWorking = 0;
}

LOCAL void ICACHE_FLASH_ATTR startResolve(const char *domain, dns_found_callback output_mode_cb, dns_found_callback instant_callback) {
	static ip_addr_t ip;
	static struct espconn connector = {0};
	err_t r = espconn_gethostbyname(&connector, domain, &ip, output_mode_cb);
	uart_send_line("Resolving...");
	if(r == ESPCONN_OK) {
		instant_callback(domain, &ip, &connector);
	} else if(r != ESPCONN_INPROGRESS) {
		uart_send_line("ERROR: illegal request");
	} else {
		mIsCommandWorking = 1;
		dhserial_set_mode(SM_OUTPUT_MODE, 0, 0);
	}
}

void ICACHE_FLASH_ATTR dhserial_commands_nslookup(const char *args) {
	startResolve(args, nslookup_cb, nslookup_res);
}

// workarounds for ping features in SDK v1.1.2
int mSent, mRecieved, mLost, mTotalDelay;
struct ping_option mCurrentPinopt = {0};
LOCAL void ICACHE_FLASH_ATTR ping_cb(void* arg, void *pdata) {
	char digitbuf[16];
	struct ping_option *pingopt = (struct ping_option *)arg;
	struct ping_resp *pingresp = (struct ping_resp *)pdata;
	if(dhserial_get_mode() == SM_OUTPUT_MODE) {
		mSent++;
		if(pingresp->ping_err == 0) {
			mRecieved++;
			mTotalDelay += pingresp->resp_time;
			snprintf(digitbuf, sizeof(digitbuf), "%d", pingresp->bytes);
			uart_send_str(digitbuf);
			uart_send_str(" bytes received from ");
			sprintIp(digitbuf, (const struct ip_addr *)&pingopt->ip);
			uart_send_str(digitbuf);
			uart_send_str(" in ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", pingresp->resp_time);
			uart_send_str(digitbuf);
			uart_send_str(" ms, icmp_seq = ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", mSent);
			uart_send_line(digitbuf);
		} else {
			mLost++;
			uart_send_str("Request timed out, icmp_seq = ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", mSent);
			uart_send_line(digitbuf);
		}
	}
}

LOCAL void ICACHE_FLASH_ATTR ping_done_cb(void* arg, void *pdata) {
	char digitbuf[16];
	struct ping_option *pingopt = (struct ping_option *)arg;
	if(dhserial_get_mode() == SM_OUTPUT_MODE) {
		uart_send_str("Total sent: ");
		snprintf(digitbuf, sizeof(digitbuf), "%d", mSent);
		uart_send_str(digitbuf);
		uart_send_str(", received: ");
		snprintf(digitbuf, sizeof(digitbuf), "%d", mRecieved);
		uart_send_str(digitbuf);
		uart_send_str(", lost: ");
		snprintf(digitbuf, sizeof(digitbuf), "%d", mLost);
		uart_send_str(digitbuf);
		if(mRecieved) {
			uart_send_str(", average time: ");
			snprintf(digitbuf, sizeof(digitbuf), "%d", mTotalDelay/mRecieved);
			uart_send_str(digitbuf);
			uart_send_line(" ms");
		} else {
			uart_send_line("");
		}
		dhserial_set_mode(SM_NORMAL_MODE, 0, 0);
	}
	mIsCommandWorking = 0;
}

LOCAL void ICACHE_FLASH_ATTR ping_nslookup_cb(const char *name, ip_addr_t *ip, void *arg) {
	if(dhserial_get_mode() == SM_OUTPUT_MODE) {
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
		} else {
			mIsCommandWorking = 0;
			dhserial_set_mode(SM_NORMAL_MODE, 0, 0);
		}
	} else {
		mIsCommandWorking = 0;
	}
}

void ICACHE_FLASH_ATTR dhserial_commands_ping(const char *args) {
	startResolve(args, ping_nslookup_cb, ping_nslookup_cb);
}
