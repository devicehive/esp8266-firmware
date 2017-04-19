/*
 * dhuart.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: uart hal for esp8266
 *
 */
#include "dhuart.h"
#include "user_config.h"
#include "dhdebug.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <user_interface.h>
#include <ets_forward.h>

#define UART_BASE 0x60000000
#define UART_INTERUPTION_STATE_REGISTER (UART_BASE + 0x08)
#define UART_INTERUPTION_ENABLE_REGISTER (UART_BASE + 0x0C)
#define UART_INTERUPTION_REGISTER (UART_BASE + 0x10)
#define UART_STATUS_REGISTER (UART_BASE + 0x1C)
#define UART_DIV_REGISTER (UART_BASE + 0x14)
#define UART_CONFIGURATION_REGISTER0 (UART_BASE + 0x20)
#define UART_CONFIGURATION_REGISTER1 (UART_BASE + 0x24)

#define DHUART_BUFFER_OVERFLOW_RESERVE INTERFACES_BUF_SIZE
LOCAL DHUART_DATA_MODE mDataMode = DUM_IGNORE;
LOCAL char mUartBuf[INTERFACES_BUF_SIZE + DHUART_BUFFER_OVERFLOW_RESERVE];
LOCAL unsigned int mUartBufPos = 0;
LOCAL os_timer_t mUartTimer;
LOCAL unsigned int mUartTimerTimeout = 250;
LOCAL unsigned char mBufInterrupt = 0;
LOCAL os_timer_t mRecoverLEDTimer;
LOCAL char mKeepLED = 0;

LOCAL ICACHE_FLASH_ATTR void arm_buf_timer(void);

LOCAL void ICACHE_FLASH_ATTR buf_timeout(void *arg) {
	unsigned int sz = mUartBufPos;
	if(sz > INTERFACES_BUF_SIZE)
		sz = INTERFACES_BUF_SIZE;
	dhuart_buf_rcv(mUartBuf, sz);
	ETS_UART_INTR_DISABLE();
	if(sz != mUartBufPos) {
		os_memmove(mUartBuf, &mUartBuf[sz], mUartBufPos - sz);
		mUartBufPos -= sz;
	} else {
		mUartBufPos = 0;
	}
	ETS_UART_INTR_ENABLE();
	if(mUartBufPos)
		arm_buf_timer();
}

LOCAL ICACHE_FLASH_ATTR void arm_buf_timer(void) {
	os_timer_disarm(&mUartTimer);
	os_timer_setfn(&mUartTimer, (os_timer_func_t *)buf_timeout, NULL);
	os_timer_arm(&mUartTimer, (mUartTimerTimeout == 0 | mUartBufPos >= INTERFACES_BUF_SIZE) ? 1 :mUartTimerTimeout, 0);
}

LOCAL void dhuart_intr_handler(void *arg) {
	if(READ_PERI_REG(UART_INTERUPTION_STATE_REGISTER) & BIT(0)) {
		const char rcvChar = READ_PERI_REG(UART_BASE) & 0xFF;
		WRITE_PERI_REG(UART_INTERUPTION_REGISTER, BIT(0));
		switch(mDataMode) {
		case DUM_PER_BYTE:
			dhuart_char_rcv(rcvChar);
			break;
		case DUM_PER_BUF:
			if(mUartBufPos >= sizeof(mUartBuf)) {
				return;
			}
			mUartBuf[mUartBufPos++] = rcvChar;
			if(mBufInterrupt) {
				if(mUartBufPos <= INTERFACES_BUF_SIZE) {
					arm_buf_timer();
				}
			}
			break;
		case DUM_IGNORE:
			break;
		}
	} else {
		WRITE_PERI_REG(UART_INTERUPTION_REGISTER, 0xffff);
	}
}

int ICACHE_FLASH_ATTR dhuart_init(unsigned int speed, unsigned int databits, char parity, unsigned int stopbits) {
	if(speed < 300 || speed > 230400 || databits < 5 || databits > 8 || !(parity == 'N' || parity == 'O' || parity == 'E') || stopbits > 2 ||  stopbits < 1)
		return 0;
	ETS_UART_INTR_DISABLE();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	gpio_output_set(0, 0, 0, BIT(1) | BIT(3));
	PIN_PULLUP_EN(PERIPHS_IO_MUX_U0RXD_U);
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);

	WRITE_PERI_REG(UART_DIV_REGISTER, UART_CLK_FREQ / speed);
	WRITE_PERI_REG(UART_CONFIGURATION_REGISTER0, 0);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, ((parity == 'E') ? 0 : 1)  <<  0);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, ((parity == 'N') ? 0 : 1)  <<  1);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, (databits - 5)  <<  2);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, ((stopbits == 2) ? 3 : 1)  <<  4);
	SET_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, BIT(17) | BIT(18));
	CLEAR_PERI_REG_MASK(UART_CONFIGURATION_REGISTER0, BIT(17) | BIT(18));
	WRITE_PERI_REG(UART_CONFIGURATION_REGISTER1, BIT(0) | BIT(16) | BIT(23));
	ETS_UART_INTR_ATTACH(dhuart_intr_handler, 0);
	WRITE_PERI_REG(UART_INTERUPTION_REGISTER, 0xffff);
	SET_PERI_REG_MASK(UART_INTERUPTION_ENABLE_REGISTER, BIT(0));
	ETS_UART_INTR_ENABLE();
	return 1;
}

LOCAL void ICACHE_FLASH_ATTR led_recover(void *arg) {
	ETS_INTR_LOCK();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
	gpio_output_set(0, 0b0110, 0b0110, 0);
	ETS_INTR_UNLOCK();
}

LOCAL void ICACHE_FLASH_ATTR led_off(void) {
	gpio_output_set(0, 0, 0, 0b0110);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	os_delay_us(10000);
}

void ICACHE_FLASH_ATTR dhuart_leds(DHUART_LEDS_MODE mode) {
	if(mode == DHUART_LEDS_ON) {
		mKeepLED = 1;
		led_recover(NULL);
	} else {
		mKeepLED = 0;
		led_off();
	}
}

LOCAL void ICACHE_FLASH_ATTR dhuart_send_char(char c) {
	while((READ_PERI_REG(UART_STATUS_REGISTER) >> 16) & 0xFF);
	WRITE_PERI_REG(UART_BASE, c);
}

void ICACHE_FLASH_ATTR dhuart_send_str(const char *str) {
	if(mDataMode == DUM_PER_BUF)
		return;
	if(mKeepLED) {
		os_timer_disarm(&mRecoverLEDTimer);
		led_off();
	}
	while(*str)
		dhuart_send_char(*str++);
	if(mKeepLED) {
		os_timer_setfn(&mRecoverLEDTimer, (os_timer_func_t *)led_recover, NULL);
		os_timer_arm(&mRecoverLEDTimer, 20, 0);
	}
}

void ICACHE_FLASH_ATTR dhuart_send_line(const char *str) {
	dhuart_send_str(str);
	dhuart_send_str("\r\n");
}

void ICACHE_FLASH_ATTR dhuart_send_buf(const char *buf, unsigned int len) {
	if(mDataMode == DUM_PER_BYTE)
		return;
	if(mKeepLED) {
		os_timer_disarm(&mRecoverLEDTimer);
		led_off();
	}
	while(len--) {
		system_soft_wdt_feed();
		dhuart_send_char(*buf++);
	}
	if(mKeepLED) {
		os_timer_setfn(&mRecoverLEDTimer, (os_timer_func_t *)led_recover, NULL);
		os_timer_arm(&mRecoverLEDTimer, 20, 0);
	}
}

void ICACHE_FLASH_ATTR dhuart_set_mode(DHUART_DATA_MODE mode) {
	mDataMode = mode;
	if(mode == DUM_PER_BUF) {
		mUartBufPos = 0;
	} else if(mode == DUM_PER_BYTE) {
		mBufInterrupt = 0;
	}
}

unsigned int ICACHE_FLASH_ATTR dhuart_get_callback_timeout(void) {
	return mUartTimerTimeout;
}

void ICACHE_FLASH_ATTR dhuart_set_callback_timeout(unsigned int timeout) {
	mUartTimerTimeout = timeout;
}

unsigned int ICACHE_FLASH_ATTR dhuart_get_buf(char ** buf) {
	*buf = mUartBuf;
	return mUartBufPos;
}

void ICACHE_FLASH_ATTR dhuart_reset_buf(void) {
	mUartBufPos = 0;
}

void ICACHE_FLASH_ATTR dhuart_enable_buf_interrupt(int enable) {
	mBufInterrupt = enable ? 1 : 0;
}
