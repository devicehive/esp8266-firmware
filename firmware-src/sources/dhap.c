/*
 * dhap.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <user_interface.h>
#include <mem.h>
#include <c_types.h>
#include "dhap.h"
#include "snprintf.h"
#include "user_config.h"
#include "dhdebug.h"
#include "dhuart.h"
#include "dhzc_dnsd.h"
#include "dhzc_web.h"

static struct ip_info *ipinfo = NULL;
struct softap_config *apconfig = NULL;

void ICACHE_FLASH_ATTR dhap_init(const char *ssid, const char *password) {
	if(!wifi_set_opmode(SOFTAP_MODE))
		dhdebug("Failed to wifi_set_opmode()");
	apconfig = (struct softap_config *)os_zalloc(sizeof(struct softap_config));;
	if(!wifi_softap_dhcps_stop())
		dhdebug("Failed to wifi_softap_dhcps_stop()");
	apconfig->ssid_len = snprintf(apconfig->ssid, sizeof(apconfig->ssid), ssid);
	apconfig->authmode = AUTH_OPEN;
	if(password) {
		if(password[0]) {
			apconfig->authmode = AUTH_WPA_WPA2_PSK;
			snprintf(apconfig->password, sizeof(apconfig->password), password);
		}
	}
	apconfig->ssid_hidden = 0;
	apconfig->channel = 7;
	apconfig->max_connection = 4;
	apconfig->beacon_interval = 100;
	if(!wifi_softap_set_config(apconfig))
		dhdebug("Failed to wifi_softap_set_config()");
	if(!wifi_set_phy_mode(PHY_MODE_11G))
		dhdebug("Failed to wifi_set_phy_mode()");
	ipinfo = (struct ip_info *)os_zalloc(sizeof(struct ip_info));
	if(!wifi_get_ip_info(SOFTAP_IF, ipinfo))
		dhdebug("Failed to wifi_get_ip_info()");
	IP4_ADDR(&ipinfo->ip, 192, 168, 2, 1);
	IP4_ADDR(&ipinfo->gw, 192, 168, 2, 1);
	IP4_ADDR(&ipinfo->netmask, 255, 255, 255, 0);
	espconn_dns_setserver(0, &ipinfo->ip);
	if(!wifi_set_ip_info(SOFTAP_IF, ipinfo))
		dhdebug("Failed to wifi_set_ip_info()");
	if(!wifi_softap_dhcps_start())
		dhdebug("Failed to wifi_softap_dhcps_start()");
	unsigned char *bip = (unsigned char *)&ipinfo->ip;
	dhdebug("Access point is initialized at %d.%d.%d.%d", bip[0], bip[1], bip[2], bip[3]);
}

void ICACHE_FLASH_ATTR dhap_zeroconf() {
	dhuart_leds(UART_LEDS_ON);
	dhdebug("\r\n**********************************\r\nWi-Fi Configuration Access Point Mode");

	dhap_init(WIFI_CONFIGURATION_SSID, NULL);

	// initialize DNS
	dhzc_dnsd_init();

	// initialize HTTP server
	dhzc_web_init();

	dhdebug("Zero configuration server is initialized");
	dhdebug("Wi-Fi SSID is %s", apconfig->ssid);
}

const struct ip_info * dhap_get_ip_info() {
	return ipinfo;
}
