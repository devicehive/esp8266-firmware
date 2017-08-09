/*
 * serialport_win.cpp
 *
 * Cross-platform serial port implementation
 *
 *  Created on: 2014
 *      Author: Nikolay Khabarov
 *  License: You can do whatever you want. Author doesn't provide warranty of any kind.
 *
 */

#include <string.h>
#include "serialport.h"

SerialPort::SerialPort(COM comport, const char *name) {
    mCom = comport;
    mTreadFlag = true;
    mReadFlag = false;
    mReadError = false;
    mBytesRecivedSinceLastSend = 0;
    mThread = 0;
    mLastReceived = 0;
    snprintf(mName, sizeof(mName), "%s", name);
}

COM SerialPort::get_com() {
    return mCom;
}

bool SerialPort::isReadError() {
	return mReadError;
}

bool SerialPort::waitAnswer(unsigned int len, unsigned int timeOutMs) {
	for(unsigned int i = 0; i < timeOutMs/10 + 1; i++)  {
		if(mBytesRecivedSinceLastSend >= len)
			return true;
		sleep(10);
	}
	return false;
}

void SerialPort::waitTransmitionEnd(unsigned int timeOutMs) {
	while(getTick() - mLastReceived < timeOutMs)
		sleep(10);
}

void SerialPort::send(const char *text) {
	mBytesRecivedSinceLastSend = 0;
	unsigned int tlen = strlen(text);
	unsigned int bw = write_native(text, tlen);
    if(bw != tlen)
        SerialPortError(this, ERROR_WRITE_STRING);
}

void SerialPort::send(char c) {
	mBytesRecivedSinceLastSend = 0;
	unsigned int bw = write_native(&c, 1);
    if(bw <= 0)
        SerialPortError(this, ERROR_WRITE_STRING);
}

void SerialPort::send(const void *data, unsigned int len) {
	mBytesRecivedSinceLastSend = 0;
	unsigned int bw = write_native(data, len);
	if(bw != len)
        SerialPortError(this, ERROR_WRITE_STRING);
}

const char *SerialPort::getName() {
    return mName;
}
