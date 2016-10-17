/*
 * mhz19.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <osapi.h>
#include <c_types.h>
#include "mhz19.h"
#include "dhuart.h"
#include "dhdebug.h"
#include "dhutils.h"

const char request[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

char * ICACHE_FLASH_ATTR mhz19_read(int *co2) {
	char *result;
	int i;
	char cs = 0;
	dhuart_set_mode(DUM_PER_BUF);
	if(dhuart_init(9600, 8, 'N', 1) == 0)
		return "Uart init failed";
	dhuart_send_buf(request, sizeof(request));
	os_delay_us(20000);
	int len = dhuart_get_buf(&result);
	if(len != 9){
		if(len)
			return "Response length mismatch";
		else
			return "No response";
		return "Response length mismatch";
	}
	dhdebug_dump(result, len);
	for(i = 1; i < 8; i++) {
		cs += result[i];
	}
	cs = ~cs;
	cs++;
	if(cs != result[8]) {
		return "Checksum mismatch";
	}
	*co2 = unsignedInt16(result, 2);
	return NULL;
}
