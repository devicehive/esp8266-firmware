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
#include "dhap.h"
#include "snprintf.h"
#include "user_config.h"
#include "dhdebug.h"
#include "dhap_httpd.h"
#include "dhap_dnsd.h"

void ICACHE_FLASH_ATTR dhap_init() {
	dhdebug_direct();
	dhdebug("\r\n**********************************\r\nWi-Fi Configuration Access Point Mode");
	// initialize Wi-Fi AP
	if(!wifi_set_opmode(SOFTAP_MODE))
		dhdebug("Failed to wifi_set_opmode()");
	struct softap_config apconfig;
	if(!wifi_softap_dhcps_stop())
		dhdebug("Failed to wifi_softap_dhcps_stop()");
	os_memset(apconfig, 0, sizeof(apconfig));
	apconfig.ssid_len = snprintf(apconfig.ssid, sizeof(apconfig.ssid), WIFI_CONFIGURATION_SSID);
	apconfig.authmode = AUTH_OPEN;
	apconfig.ssid_hidden = 0;
	apconfig.channel = 7;
	apconfig.max_connection = 4;
	apconfig.beacon_interval = 100;
	if(!wifi_softap_set_config(&apconfig))
		dhdebug("Failed to wifi_softap_set_config()");
	if(!wifi_set_phy_mode(PHY_MODE_11G))
		dhdebug("Failed to wifi_set_phy_mode()");
	struct ip_info *ipinfo = (struct ip_info *)os_zalloc(sizeof(struct ip_info));
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

	// initialize DNS
	dhap_dnsd_init();

	// initialize HTTP server
	dhap_httpd_init();

	unsigned char *bip = (unsigned char *)&ipinfo->ip;
	dhdebug("Zero configuration server initialized at %d.%d.%d.%d", bip[0], bip[1], bip[2], bip[3]);
	dhdebug("Wi-Fi SSID is %s", apconfig.ssid);
}
