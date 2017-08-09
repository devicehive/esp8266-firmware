/*
 * custom_firmware.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "dhrequest.h"
#include "dhdebug.h"

#include <c_types.h>

HTTP_REQUEST * ICACHE_FLASH_ATTR custom_firmware_request(void) {
	// reimplement this method to return actual HTTP_REQUEST to make custom notification firmware
	return 0;
}
