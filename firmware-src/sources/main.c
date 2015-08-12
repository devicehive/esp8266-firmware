/*
 * user_main.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: DeviceHive firmware for ESP8266
 *
 */

#include <osapi.h>

#include "dhcommands.h"
#include "dhdebug.h"
#include "dhterminal.h"
#include "dhsettings.h"

extern int rtc_mem_check(int f);

void user_rf_pre_init(void) {
	system_restore();
	rtc_mem_check(0);
}

void user_init(void) {
	dhterminal_init();
	dhdebug("*****************************");
	dhsettings_init();
	dhconnector_init(dhcommands_do);
	dhdebug("Initialization complete");
}
