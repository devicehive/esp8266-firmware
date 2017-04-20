/*
 * mhz19.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */
#include "mhz19.h"
#include "DH/uart.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>
#include <c_types.h>

const char request[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

char * ICACHE_FLASH_ATTR mhz19_read(int *co2) {
	char *result;
	int i;
	char cs = 0;
	dh_uart_set_mode(DH_UART_MODE_PER_BUF);
	if(dh_uart_init(9600, 8, 'N', 1) != 0)
		return "failed to init UART";
	dh_uart_send_buf(request, sizeof(request));
	delay_ms(20);
	size_t len = dh_uart_get_buf((void**)&result);
	if(len != 9){
		if(len)
			return "Response length mismatch";
		return "No response";
	}
	for(i = 1; i < 8; i++) {
		cs += result[i];
	}
	cs = ~cs;
	cs++;
	if(cs != result[8]) {
		return "Checksum mismatch";
	}
	*co2 = unsignedInt16be(result, 2);
	return NULL;
}
