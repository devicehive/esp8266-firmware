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
#include "dhmem.h"

#include <ets_sys.h>
#include <osapi.h>
#include <ets_forward.h>

static int mGlobalBlock = 0;

void ICACHE_FLASH_ATTR dhmem_block(void) {
	mGlobalBlock = 1;
	ETS_GPIO_INTR_DISABLE();
}

void ICACHE_FLASH_ATTR dhmem_unblock(void) {
	mGlobalBlock = 0;
	dhmem_unblock_cb();
	ETS_GPIO_INTR_ENABLE();
}

int ICACHE_FLASH_ATTR dhmem_isblock(void) {
	return mGlobalBlock;
}
