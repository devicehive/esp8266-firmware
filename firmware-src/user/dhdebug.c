/*
 * dhdebug.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for debug output
 *
 */
#include <stdarg.h>
#include <c_types.h>
#include "dhdebug.h"
#include "dhserial.h"

void ICACHE_FLASH_ATTR dhdebug(const char *fmt, ...) {
	va_list    ap;
	va_start(ap, fmt);
	dhserial_debug(fmt, ap);
	va_end(ap);
}
