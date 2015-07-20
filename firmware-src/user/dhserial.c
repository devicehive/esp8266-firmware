/*
 * dhserial.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: UART terminal implementation
 *
 */

#include <stdarg.h>
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include "dhserial.h"
#include "user_config.h"
#include "snprintf.h"
#include "dhserial_commandline.h"
#include "dhserial_commands.h"

#define DEBUG_BUFF_SPACE_FOR_ONE_LINE 128
DHSERIAL_MODE mMode = SM_NORMAL_MODE;
char mDebugBuff[1024] = {0};
int mDebugBuffPos = 0;
char mRcvBuff[79];
int mRcvBuffPos = 0;
char mEscSequence[4] = {0};
int mEscSequencePos = 0;
int mEscRecieving = 0;
char mHistoryBuff[1024] = {0};
int mHistoryBuffPos = 0;
int mHistoryPrevPos = 0;
int mHistoryScrollBufPos = 0;
char mHistoryRcvBuff[sizeof(mRcvBuff)] = {0};
char mHistoryRcvBuffFill = 0;
Input_Call_Back mInputCallBack = dhserial_commandline_do;
Input_Autocompleater mCompleaterCallBack = dhserial_commandline_autocompleater;
os_timer_t mAwatingTimer;

LOCAL void ICACHE_FLASH_ATTR printWelcome() {
	if (mMode == SM_NORMAL_MODE) {
		uart_send_str("$ ");
	} else if(mMode == SM_INPUT_MODE || mMode == SM_HIDDEN_INPUT_MODE) {
		uart_send_str("> ");
	}
	mRcvBuffPos = 0;
	os_memset(mRcvBuff, 0, sizeof(mRcvBuff));
}

void ICACHE_FLASH_ATTR dhserial_set_input(const char *line) {
	uart_send_str("\r\x1B[K");
	printWelcome();
	mRcvBuffPos = snprintf(mRcvBuff, sizeof(mRcvBuff), "%s", line);
	uart_send_str(mRcvBuff);
}

const char * ICACHE_FLASH_ATTR dhserial_get_history() {
	return mHistoryBuff;
}

const char * ICACHE_FLASH_ATTR dhserial_get_debug_ouput() {
	return mDebugBuff;
}

DHSERIAL_MODE ICACHE_FLASH_ATTR dhserial_get_mode() {
	return mMode;
}

void ICACHE_FLASH_ATTR dhserial_set_mode(DHSERIAL_MODE mode, Input_Call_Back inputcb, Input_Autocompleater complcb) {
	mMode = mode;
	if(mMode != SM_AWATING_MODE)
		printWelcome();
	if(mMode == SM_INPUT_MODE || mMode == SM_HIDDEN_INPUT_MODE) {
		mInputCallBack = inputcb;
		mCompleaterCallBack = complcb;
	} else {
		mInputCallBack = dhserial_commandline_do;
		mCompleaterCallBack = dhserial_commandline_autocompleater;
	}
}

LOCAL void ICACHE_FLASH_ATTR trimRcvBuff() {
	char *pos = mRcvBuff;
	char *from = mRcvBuff;
	char last = ' ';
	char quote = 0;
	while(last) {
		if(quote == 0 && last == ' ' && *from == ' ') {
			from++;
		} else {
			last = *from++;
			*pos++ = last;
			if(last =='"'|| last == '\'') {
				quote = (quote == last) ? 0 : last;
			}
		}
	}
	pos -= 2;
	if(pos > mRcvBuff && quote == 0)
		if(*pos == ' ')
			*pos = 0;
}

LOCAL void ICACHE_FLASH_ATTR do_command(void *arg) {
	if(dhserial_commands_is_busy()) {
		dhserial_set_mode(SM_AWATING_MODE, 0, 0);
		os_timer_disarm(&mAwatingTimer);
		os_timer_setfn(&mAwatingTimer, (os_timer_func_t *)do_command, NULL);
		os_timer_arm(&mAwatingTimer, 100, 0);
		return;
	} else if(mMode == SM_AWATING_MODE) {
		mMode = SM_NORMAL_MODE;
	}
	const DHSERIAL_MODE mode = mMode;
	if(mode == SM_NORMAL_MODE) {
		trimRcvBuff();
		if(mRcvBuff[0] && os_strcmp(mRcvBuff, &mHistoryBuff[mHistoryPrevPos]) != 0 ) {
			mHistoryPrevPos = mHistoryBuffPos;
			mHistoryBuffPos += snprintf(&mHistoryBuff[mHistoryBuffPos], sizeof(mHistoryBuff) - mHistoryBuffPos - 1, "%s", mRcvBuff) + 1;
			mHistoryBuff[mHistoryBuffPos + 1] = 0; // mark end with double null terminating
			int n =  sizeof(mRcvBuff) - (sizeof(mHistoryBuff) - mHistoryBuffPos - 1);
			if(n > 0) {
				while(mHistoryBuff[n - 1])
					n++;
				os_memmove(mHistoryBuff, &mHistoryBuff[n], sizeof(mHistoryBuff) - n);
				mHistoryBuffPos -= n;
				mHistoryPrevPos = (mHistoryPrevPos < n) ? 0 : (mHistoryPrevPos - n);
			}
		}
		mHistoryScrollBufPos = mHistoryBuffPos;
		mHistoryRcvBuffFill = 0;
	}
	mInputCallBack(mRcvBuff);
	if(mode == SM_NORMAL_MODE && mMode == SM_NORMAL_MODE) {
		printWelcome();
	}
}

void ICACHE_FLASH_ATTR uart_char_rcv(char c) {
	static char lastRecieved = 0;
	if(c == '\n' && lastRecieved=='\r') {
		lastRecieved = c;
		return;
	}
	lastRecieved = c;
	if(c == '\r')
		c = '\n';
	int i;
	if(mMode == SM_DEBUG_MODE || mMode == SM_OUTPUT_MODE || mMode == SM_AWATING_MODE) {
		if(c == 'q' || c == 0x3 /*Ctrl+C*/) {
			if(mMode == SM_DEBUG_MODE) {
				mDebugBuff[0] = 0;
				mDebugBuffPos = 0;
			} else if (mMode == SM_AWATING_MODE) {
				uart_send_str("^C\r\n");
				os_timer_disarm(&mAwatingTimer);
				mHistoryScrollBufPos = mHistoryBuffPos;
				mHistoryRcvBuffFill = 0;
			}
			dhserial_set_mode(SM_NORMAL_MODE, 0, 0);
		} else if(c == '\n' && mMode == SM_DEBUG_MODE) {
			uart_send_str("\r\n");
		}
		return;
	}

	if(mEscRecieving) {
		if(mEscSequencePos < sizeof(mEscSequence) - 1) {
			mEscSequence[mEscSequencePos++] = c;
			mEscSequence[mEscSequencePos] = 0;
		} else {
			mEscSequence[0] = 0;
		}
		if(mEscSequencePos > 1 && c >= 0x40 && c <= 0x7e) {
			mEscRecieving = 0;
			mEscSequencePos = 0;
			if(os_strcmp(mEscSequence, "[A") == 0 && mMode == SM_NORMAL_MODE) { // Up
				if(mHistoryScrollBufPos > 1) {
					if(mHistoryScrollBufPos == mHistoryBuffPos) { // store current line
						snprintf(mHistoryRcvBuff, sizeof(mHistoryRcvBuff), "%s", mRcvBuff);
						mHistoryRcvBuffFill = 1;
					}
					mHistoryScrollBufPos--;
					while(mHistoryBuff[mHistoryScrollBufPos-1]) {
						mHistoryScrollBufPos--;
						if(mHistoryScrollBufPos == 0)
							break;
					}
					dhserial_set_input(&mHistoryBuff[mHistoryScrollBufPos]);
				}
			} else if(os_strcmp(mEscSequence, "[B") == 0 && mMode == SM_NORMAL_MODE) { // Down
				int m = 0;
				while(mHistoryBuff[mHistoryScrollBufPos]) {
					mHistoryScrollBufPos++;
					m = 1;
				}
				if(m)
					mHistoryScrollBufPos++;
				if(mHistoryBuff[mHistoryScrollBufPos]) {
					dhserial_set_input(&mHistoryBuff[mHistoryScrollBufPos]);
				} else if(mHistoryRcvBuffFill) {
					dhserial_set_input(mHistoryRcvBuff);
					mHistoryRcvBuffFill = 0;
				}
			} else if(os_strcmp(mEscSequence, "[C") == 0) { // Right
				if(mRcvBuff[mRcvBuffPos]) {
					mRcvBuffPos++;
					uart_send_char(0x1B);
					uart_send_str(mEscSequence);
				}
			} else if(os_strcmp(mEscSequence, "[D") == 0) { // Left
				if(mRcvBuffPos) {
					mRcvBuffPos--;
					uart_send_char(0x1B);
					uart_send_str(mEscSequence);
				}
			} else if(os_strcmp(mEscSequence, "[3~") == 0) { // Delete
				if(mRcvBuff[mRcvBuffPos])
					uart_send_str("\x1B[1P");
				for(i = mRcvBuffPos + 1; i < sizeof(mRcvBuff); i++) {
					if(mRcvBuff[i - 1] == 0)
						break;
					mRcvBuff[i - 1] = mRcvBuff[i];
				}
			}
		}
		return;
	}

	if(c == '\n') {
		uart_send_str("\r\n");
		do_command(0);
	} else {
		if(c == 0x09) { // Tab
			if(mCompleaterCallBack) {
				char * a = mRcvBuff;
				while(*a == ' ')
					a++;
				a = mCompleaterCallBack(a);
				if(a)
					dhserial_set_input(a);
			}
		} else if(c == 0x03) { // Ctrl+C
			uart_send_str("^C\r\n");
			dhserial_set_mode(SM_NORMAL_MODE, 0, 0);
			mHistoryScrollBufPos = mHistoryBuffPos;
			mHistoryRcvBuffFill = 0;
			mRcvBuffPos = 0;
			os_memset(mRcvBuff, 0, sizeof(mRcvBuff));
		} else if(c == 0x1B) { // ESC character
			mEscRecieving = 1;
		} else if(c == 0x7F || c == 0x08) { // backspace
			if(mRcvBuffPos == 0)
				return;
			for(i = mRcvBuffPos; i < sizeof(mRcvBuff); i++) {
				mRcvBuff[i - 1] = mRcvBuff[i];
				if(mRcvBuff[i] == 0)
					break;
			}
			mRcvBuffPos--;
			uart_send_str("\x1B[D\x1B[1P");
		} else if(mRcvBuff[sizeof(mRcvBuff) - 2] == 0) { // if we have space
			if(c > 0x1F) {
				if(mRcvBuff[mRcvBuffPos] !=0)
					uart_send_str("\x1B[@");
				if(mMode == SM_HIDDEN_INPUT_MODE) {
					uart_send_char('*');
				} else {
					uart_send_char(c);
				}
				char next = c;
				for(i = mRcvBuffPos; i < sizeof(mRcvBuff); i++) {
					char tmp = next;
					next = mRcvBuff[i];
					mRcvBuff[i] = tmp;
					if(tmp == 0)
						break;
				}
				mRcvBuffPos++;
			}
		}
	}
}

void ICACHE_FLASH_ATTR dhserial_debug(const char *pFormat, va_list ap) {
	int len = vsnprintf(&mDebugBuff[mDebugBuffPos], sizeof(mDebugBuff) - mDebugBuffPos - 2, pFormat, ap);
	if(mMode == SM_DEBUG_MODE) {
		uart_send_line(&mDebugBuff[mDebugBuffPos]);
	} else {
		mDebugBuffPos += len;
		mDebugBuff[mDebugBuffPos++] = '\r';
		mDebugBuff[mDebugBuffPos++] = '\n';
		mDebugBuff[mDebugBuffPos] = 0;
		int n =  DEBUG_BUFF_SPACE_FOR_ONE_LINE - (sizeof(mDebugBuff) - mDebugBuffPos - 2);
		if(n > 0) {
			while(mDebugBuff[n - 1] != '\n')
				n++;
			os_memmove(mDebugBuff, &mDebugBuff[n], sizeof(mDebugBuff) - n);
			mDebugBuffPos -= n;
		}
	}
}

void ICACHE_FLASH_ATTR dhserial_init() {
	uart_init(UART_BAUND_RATE, 0);
	system_set_os_print(0);
	uart_send_str("\r\n**********************************\r\nUart terminal ready.\r\n");
	printWelcome();
}
