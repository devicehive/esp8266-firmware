/*
 * dhadc.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include "dhadc.h"

int ICACHE_FLASH_ATTR dhadc_get_value(){
	return system_adc_read();
}
