/*
 * custom_firmware.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include "dhdebug.h"
#include "dhgpio.h"
#include "dhrequest.h"
#include "dhconnector.h"
#include "snprintf.h"
#include "c_types.h"
#include "osapi.h"

HTTP_REQUEST * ICACHE_FLASH_ATTR custom_firmware_request() {
	// reimplement this method to return actual HTTP_REQUEST to make custom notification firmware
	return 0;
}
