/*
 * dhmem.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Memory observer module
 *
 */

#define DHREQUEST_RAM_RESERVE 7168
#define DHRECOVER_RAM_SIZE 12288

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <mem.h>
#include "dhmem.h"
#include "dhdebug.h"

unsigned int mGlobalBlock = 0;

HTTP_REQUEST * ICACHE_FLASH_ATTR dhmem_malloc_request(unsigned int size) {
	HTTP_REQUEST * const res = (HTTP_REQUEST *)os_malloc(size);
	if (system_get_free_heap_size() < DHREQUEST_RAM_RESERVE) {
		mGlobalBlock = 1;
		ETS_GPIO_INTR_DISABLE();
	}
	return res;
}

void ICACHE_FLASH_ATTR dhmem_free_request(HTTP_REQUEST *r) {
	os_free(r);
	if(mGlobalBlock) {
		if (system_get_free_heap_size() > DHRECOVER_RAM_SIZE) {
			mGlobalBlock = 0;
			dhmem_unblock();
			ETS_GPIO_INTR_ENABLE();
		}
	}
}

int ICACHE_FLASH_ATTR dhmem_isblock() {
	return mGlobalBlock;
}
