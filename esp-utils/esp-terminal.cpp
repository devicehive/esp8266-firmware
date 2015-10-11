/*
 * main.cpp
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Main file of esp-terminal
 *
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "common/serialport.h"
#include "common/terminal.h"

Terminal *term;

#define AUTODETECT_MAX_PORT 20
#define AUTODETECT_SEND_STRING "\x03"
#define AUTODETECT_STRING "^C\r\n$ "
#define AUTODETECT_TIMEOUT 1000

void SerialPortError(SerialPort */*port*/, const char *text)
{
	term->put((char*)"\r\n");
    term->put(text);
}

int recieved = 0;
char detectBuf[sizeof(AUTODETECT_STRING)] = {0};
unsigned int detectBufPos = 0;
bool isWorking = false;
void SerialPortRecieved(SerialPort */*port*/, const char *text,  unsigned int len)
{
	if(isWorking) {
		while(recieved < 4 && *text) {
			text++;
			recieved++;
		}
		if(*text)
			term->put(text);
	} else {
		for (unsigned int i = 0; i < len; i++) {
			if(detectBufPos >= sizeof(detectBuf) - 1) {
				for(unsigned int j = 0; j < sizeof(detectBuf) - 1; j++) {
					detectBuf[j] = detectBuf[j + 1];
				}
				detectBuf[sizeof(detectBuf) - 2] = text[i];
			} else {
				detectBuf[detectBufPos++] = text[i];
			}
		}
	}
}

SerialPort *detectPort(){
    SerialPort *res;
    printf("Detecting device...\r\n");
    for(int i = 0; i < AUTODETECT_MAX_PORT; i++) {
    	const char *name = SerialPort::findNextPort(false);
    	if(name[0] == 0)
    		break;
    	res = SerialPort::open(name);
        if( res ) {
        	res->send(AUTODETECT_SEND_STRING);
        	if(!res->waitAnswer(sizeof(detectBuf) - 1, AUTODETECT_TIMEOUT)) {
        		delete res;
        		continue;
        	}
        	if(strcmp(detectBuf, AUTODETECT_STRING) == 0)
            {
                printf("Terminal device found on %s\r\n", name);
                SerialPort::findNextPort(true);
                return res;
            }
            else
                delete res;
        }
    }
    SerialPort::findNextPort(true);
    printf("Terminal device not found.\r\n");
    return 0;
}

int main(int argc, char* argv[]) {
	setbuf(stdout, NULL);
	SerialPort *port;
	if(argc > 1) {
		if(argc > 2) {
			printf("Unknown args, only port name is acceptable. Press ENTER for exit.\r\n");
			getchar();
			return 1;
		}
		port = SerialPort::open(argv[1]);
		if(!port) {
				printf("Can not open port %s. Press ENTER for exit.\r\n", argv[1]);
				getchar();
				return 1;
		}
	} else {
		port = detectPort();
		if(!port) {
			printf("Can not detect port. Check if device connected and driver is installed.\r\n" \
					"If port number greater than %d, specify port manualy as arg.\r\n" \
					"Press ENTER for exit\r\n", AUTODETECT_MAX_PORT);
			getchar();
			return 1;
		}
	}
	printf("Port open. Press Ctrl+Q for exit.\r\n");
	term = Terminal::open();

	isWorking = true;
	port->send((char*)"\x03"); // reset terminal
	while(true) {
		if(port->isReadError())
			break;
		char c[5];
		term->get(c);
		if (c[0] == 0x11) {
			printf("\r\n");
			break;
		}
		port->send(c);
	}

	delete port;
	delete term;
	return 0;
}


