/*
 * rest.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include <c_types.h>
#include <spi_flash.h>
#include <osapi.h>
#include <os_type.h>
#include <mem.h>
#include "irom.h"
#include "uploadable_page.h"
#include "dhdebug.h"

/** Uploadable web page maximum length and ROM addresses. */
#define UPLOADABLE_PAGE_START_SECTOR 0x68
#define UPLOADABLE_PAGE_END_SECTOR 0x77
#define UPLOADABLE_PAGE_MAX_SIZE ((UPLOADABLE_PAGE_END_SECTOR - UPLOADABLE_PAGE_START_SECTOR + 1) * SPI_FLASH_SEC_SIZE)
/** Timeout for auto finish. */
#define UPLOADABLE_PAGE_TIMEOUT_MS 60000

LOCAL unsigned int mPageLength = 0;
LOCAL unsigned int mFlashingSector = 0;
LOCAL os_timer_t mFlashingTimer;
LOCAL char *mBuffer = NULL;
LOCAL unsigned int mBufferPos = 0;

LOCAL void ICACHE_FLASH_ATTR flash_timeout(void *arg) {
	dhdebug("Flashing procedure isn't finished correctly, force to finish");
	uploadable_page_finish();
}

LOCAL void ICACHE_FLASH_ATTR reset_timer() {
	os_timer_disarm(&mFlashingTimer);
	os_timer_setfn(&mFlashingTimer, (os_timer_func_t *)flash_timeout, NULL);
	os_timer_arm(&mFlashingTimer, UPLOADABLE_PAGE_TIMEOUT_MS, 0);
}

const char *ICACHE_FLASH_ATTR uploadable_page_get(unsigned int *len) {
	RO_DATA char flashing[] = "<html><body><h1>Flashing is in process...</h1></body></html>";
	if(mBuffer) {
		*len = sizeof(flashing) - 1;
		return flashing;
	}
	const uint32_t * const dwdata = (const uint32_t*)(IROM_FLASH_BASE_ADDRESS +
			UPLOADABLE_PAGE_START_SECTOR * SPI_FLASH_SEC_SIZE);
	if(mPageLength == 0) {
		unsigned int i;
		for(i = 0; i < UPLOADABLE_PAGE_MAX_SIZE; i += sizeof(uint32_t)) {
			uint32_t b = dwdata[i / sizeof(uint32_t)];
			if((b & 0x000000FF) == 0) {
				break;
			} else if((b & 0x0000FF00) == 0) {
				i++;
				break;
			} else if((b & 0x00FF0000) == 0) {
				i += 2;
				break;
			} else if((b & 0xFF000000) == 0) {
				i += 3;
				break;
			}
		}
		mPageLength = i;
	}
	*len = mPageLength;
	return (const char *)dwdata;
}

LOCAL SpiFlashOpResult ICACHE_FLASH_ATTR write_zero_byte(unsigned int sector) {
	if(sector > UPLOADABLE_PAGE_END_SECTOR) {
		return SPI_FLASH_RESULT_ERR;
	}
	uint8_t data[4];
	irom_read(data, (char *)(IROM_FLASH_BASE_ADDRESS + sector * SPI_FLASH_SEC_SIZE), sizeof(data));
	// since zero is going to be written here, there is no need in erase operation
	data[0] = 0;
	return spi_flash_write(sector * SPI_FLASH_SEC_SIZE, (uint32 *)data, sizeof(data));
}

UP_STATUS ICACHE_FLASH_ATTR uploadable_page_delete() {
	// set first byte to zero
	if(write_zero_byte(UPLOADABLE_PAGE_START_SECTOR) == SPI_FLASH_RESULT_OK)
		return UP_STATUS_OK;
	return UP_STATUS_INTERNAL_ERROR;
}

UP_STATUS ICACHE_FLASH_ATTR uploadable_page_begin() {
	ETS_INTR_LOCK();
	if(mBuffer == NULL) {
		mBuffer = (char *)os_malloc(SPI_FLASH_SEC_SIZE);
	}
	mBufferPos = 0;
	mFlashingSector = UPLOADABLE_PAGE_START_SECTOR;
	ETS_INTR_UNLOCK();
	if(mBuffer == NULL) {
		dhdebug("No memory to initialize page flashing");
		return UP_STATUS_INTERNAL_ERROR;
	}
	reset_timer();
	dhdebug("Page flashing is initialized");
	return UP_STATUS_OK;
}

LOCAL UP_STATUS ICACHE_FLASH_ATTR flash_data() {
	if(mFlashingSector > UPLOADABLE_PAGE_END_SECTOR) {
		return UP_STATUS_OVERFLOW;
	}

	if((SPI_FLASH_SEC_SIZE - mBufferPos) > 0) {
		irom_read(&mBuffer[mBufferPos], (const char *)
				(mFlashingSector * SPI_FLASH_SEC_SIZE + mBufferPos + IROM_FLASH_BASE_ADDRESS),
				SPI_FLASH_SEC_SIZE - mBufferPos);
		mBufferPos = SPI_FLASH_SEC_SIZE;
	}

	if(irom_cmp(mBuffer, (const char *)
			(mFlashingSector * SPI_FLASH_SEC_SIZE + IROM_FLASH_BASE_ADDRESS),
			SPI_FLASH_SEC_SIZE) == 0) {
		return UP_STATUS_OK;
	}

	SpiFlashOpResult res;
	res = spi_flash_erase_sector(mFlashingSector);
	if(res == SPI_FLASH_RESULT_OK) {
		dhdebug("Flashing page at address 0x%X", mFlashingSector * SPI_FLASH_SEC_SIZE);
		res = spi_flash_write(mFlashingSector * SPI_FLASH_SEC_SIZE,
				(uint32 *)mBuffer, SPI_FLASH_SEC_SIZE);
	}

	system_soft_wdt_feed();
	if(res == SPI_FLASH_RESULT_OK) {
		mFlashingSector++;
		return UP_STATUS_OK;
	}
	return UP_STATUS_INTERNAL_ERROR;
}

UP_STATUS ICACHE_FLASH_ATTR uploadable_page_put(const char *data, unsigned int data_len) {
	if(mBuffer == NULL)
		return UP_STATUS_WRONG_CALL;
	if(mFlashingSector > UPLOADABLE_PAGE_END_SECTOR)
		return UP_STATUS_OVERFLOW;
	reset_timer();

	ETS_INTR_LOCK();
	while(data_len) {
		uint32_t tocopy = (data_len > (SPI_FLASH_SEC_SIZE - mBufferPos)) ?
				(SPI_FLASH_SEC_SIZE - mBufferPos): data_len;
		os_memcpy(&mBuffer[mBufferPos], data, tocopy);
		mBufferPos += tocopy;
		data_len -= tocopy;
		data += tocopy;
		if(mBufferPos == SPI_FLASH_SEC_SIZE) {
			SpiFlashOpResult res = flash_data();
			mBufferPos = 0;
			if(res != SPI_FLASH_RESULT_OK) {
				ETS_INTR_UNLOCK();
				dhdebug("Error while writing page at 0x%X",
						mFlashingSector * SPI_FLASH_SEC_SIZE);
				return UP_STATUS_INTERNAL_ERROR;
			}
		}
	}
	ETS_INTR_UNLOCK();
	return UP_STATUS_OK;
}

UP_STATUS ICACHE_FLASH_ATTR uploadable_page_finish() {
	if(mBuffer == NULL)
		return UP_STATUS_WRONG_CALL;
	os_timer_disarm(&mFlashingTimer);
	ETS_INTR_LOCK();
	// Mark data with null terminated char. If data takes whole available space,
	// there is no need in null terminated char.
	// At this point buffer should have at lest 1 free byte, since buffer
	// reaches maximum size before, it had to be written to flash.
	// If nothing was written, just report ok.
	SpiFlashOpResult res = SPI_FLASH_RESULT_OK;
	if(mBufferPos) {
		mBuffer[mBufferPos] = 0;
		mBufferPos++;
		res = flash_data();
	} else if(mFlashingSector > UPLOADABLE_PAGE_START_SECTOR &&
			mFlashingSector <= UPLOADABLE_PAGE_END_SECTOR) {
		res = write_zero_byte(mFlashingSector);
	}
	mBufferPos = 0;
	os_free(mBuffer);
	mBuffer = NULL;
	// force to recalc page size
	mPageLength = 0;
	ETS_INTR_UNLOCK();
	if(res != SPI_FLASH_RESULT_OK) {
		dhdebug("Error while finishing flash page");
		return UP_STATUS_INTERNAL_ERROR;
	}
	dhdebug("Flashing page has finished successfully");
	return UP_STATUS_OK;
}
