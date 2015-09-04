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
#include "dhmem.h"
#include "dhdebug.h"
#include "dhsender_queue.h"

LOCAL unsigned int mGlobalBlock = 0;

void ICACHE_FLASH_ATTR dhmem_block() {
	mGlobalBlock = 1;
	ETS_GPIO_INTR_DISABLE();
}

void ICACHE_FLASH_ATTR dhmem_unblock() {
	mGlobalBlock = 0;
	dhmem_unblock_cb();
	ETS_GPIO_INTR_ENABLE();
}

int ICACHE_FLASH_ATTR dhmem_isblock() {
	return mGlobalBlock;
}
