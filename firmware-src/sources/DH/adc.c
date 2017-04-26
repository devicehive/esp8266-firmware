/**
 * @file
 * @brief ADC hardware access layer for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "DH/adc.h"

#include <ets_sys.h>
#include <os_type.h>
#include <osapi.h>
#include <user_interface.h>
#include <ets_forward.h>

// module variables
static os_timer_t mTimer;

/*
 * dh_adc_get_value() implementation.
 */
float ICACHE_FLASH_ATTR dh_adc_get_value(void)
{
	return system_adc_read() / 1024.0f;
}


/**
 * @brief Timeout callback.
 */
static void ICACHE_FLASH_ATTR timeout_cb(void *arg)
{
	// call external handler
	dh_adc_loop_value_cb(dh_adc_get_value());
}


/*
 * dh_adc_loop() implementation.
 */
void ICACHE_FLASH_ATTR dh_adc_loop(unsigned int period_ms)
{
	os_timer_disarm(&mTimer);
	if (period_ms) {
		os_timer_setfn(&mTimer, timeout_cb, NULL);
		os_timer_arm(&mTimer, period_ms, 1);
	}
}
