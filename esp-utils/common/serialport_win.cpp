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

#include "serialport.h"
#include <stdio.h>
#ifdef COMWINDOWS
#include <windows.h>

DWORD SerialPort::ThreadProc (LPVOID lpdwThreadParam ) {
    SerialPort *port =  (SerialPort *)lpdwThreadParam;
    HANDLE hCOM = port->get_com();
    DWORD read;
    const int buffSize = 1024;
    char buff[buffSize];
    while (true) {
        read = 0;
        bool res = ReadFile(hCOM, buff, buffSize, &read,0);
        if(!port->mTreadFlag)
        	break;
        if(res) {
            if(read > 0) {
                port->mReadError = false;
                buff[read]='\0';
                SerialPortRecieved(port, buff, read);
                port->mBytesRecivedSinceLastSend += read;
            } else {
                if(GetCommModemStatus(hCOM, &read)==0) {
                    if(port->mReadError == false)
                        SerialPortError(port, ERROR_READ_STRING);
                    port->mReadError = true;
                } else {
                    port->mReadError = false;
                }
                port->sleep(100);
            }
        } else {
            port->mReadError = true;
            SerialPortError(port, ERROR_READ_STRING);
            port->sleep(100);
        }
    }
    return 0;
}

SerialPort *SerialPort::open(const char *port) {
    char namebuff[MAX_PATH];
    snprintf(namebuff, sizeof namebuff, "\\\\.\\%s", port);
    HANDLE hCOM=CreateFileA(namebuff,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
    if (hCOM!=INVALID_HANDLE_VALUE)
    {
        DCB cdcb;
        if( GetCommState(hCOM,&cdcb)==0 ) {
            CloseHandle(hCOM);
            return 0;
        }
        cdcb.BaudRate=CBR_115200;
        cdcb.Parity=NOPARITY;
        cdcb.ByteSize=8;
        cdcb.StopBits=ONESTOPBIT;
        cdcb.EvtChar=13;
        if( SetCommState(hCOM,&cdcb)==0 ) {
            CloseHandle(hCOM);
            return 0;
        }
//        SetCommMask(hCOM,0);
//        SetupComm(hCOM, 10240,10240);
//        PurgeComm(hCOM, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
        COMMTIMEOUTS to;
        to.ReadIntervalTimeout = MAXDWORD;
        to.ReadTotalTimeoutMultiplier = 0;
        to.ReadTotalTimeoutConstant = 0;
        to.WriteTotalTimeoutMultiplier = 0;
        to.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(hCOM,&to);

        SerialPort *port = new SerialPort(hCOM);
        DWORD dwThreadId;
        if ( (port->mThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ThreadProc, (LPVOID)port,  0, &dwThreadId)) == NULL)
        {
            delete port;
            return 0;
        }
        return port;
    }
    return 0;
}

SerialPort::~SerialPort() {
    mTreadFlag = false;
    CloseHandle(mCom);
    WaitForSingleObject((HANDLE)mThread, 1000);
}

unsigned int SerialPort::write_native(const void *data, unsigned int len) {
	DWORD nb;
	DWORD total = 0;
	do {
		nb = 0;
		WriteFile(mCom, (char*)data + total, len - total, &nb, 0);
		total += nb;
		if(nb)
			FlushFileBuffers(mCom);
	} while(nb > 0 && total < len);
	return total;
}

void SerialPort::sleep(unsigned int ms) {
	Sleep(ms);
}

const char *SerialPort::findNextPort(bool finish) {
	static char buf[32];
	static unsigned int num = 0;
	snprintf(buf, sizeof(buf), "COM%d", num++);
	if(finish)
		num = 0;
	return buf;
}

#endif
