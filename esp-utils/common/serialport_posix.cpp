/*
 * serialport_posix.cpp
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
#ifdef COMPOSIX

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/ioctl.h>

void * SerialPort::thread_start(void *arg) {
    SerialPort *port =  (SerialPort *)arg;
    int hCOM = port->get_com();

    const int buffSize = 1024;
    char buff[buffSize];
    while (true) {
        int rb = read(hCOM, buff, buffSize);
        port->mReadFlag = true;
        if(!port->mTreadFlag)
        	break;
        if(rb>0) {
        	port->mLastReceived = port->getTick();
            port->mReadError = false;
            buff[rb]='\0';
            SerialPortRecieved(port, buff, rb);
            port->mBytesRecivedSinceLastSend += rb;
        } else if(rb==0) {
            const bool w = port->mReadError;
            port->mReadError = true;
            if(w == false)
                SerialPortError(port, ERROR_READ_STRING);
            port->sleep(10);
        } else {
            port->mReadError = false;
            port->sleep(10);
        }
    }
    return 0;
}

SerialPort *SerialPort::open(const char *port) {
    COM comp = ::open ( port, O_RDWR | O_NONBLOCK | O_NDELAY);
    if(comp>=0)
    {
       struct termios tio;
       memset(&tio,0,sizeof(tio));
       if( tcgetattr ( comp, &tio ) != 0 ) {
           close(comp);
           return 0;
       }
       tio.c_iflag = 0;
       tio.c_oflag = 0;
       tio.c_lflag = 0;
       tio.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
       tio.c_cflag = CS8|CREAD|CLOCAL;
       tio.c_cc[VMIN] = 1;
       tio.c_cc[VTIME] = 5;

       cfsetospeed(&tio,B115200);
       cfsetispeed(&tio,B115200);

       tcflush(comp, TCIOFLUSH);

       if( tcsetattr ( comp, TCSANOW, &tio ) != 0) {
           close(comp);
           return 0;
       }

       SerialPort *comport = new SerialPort(comp, port);
       pthread_t  thr;
       if(pthread_create(&thr, 0, thread_start, (void *)comport)!=0) {
           delete comport;
           return 0;
       }
       comport->mThread = (void *)thr;
       while(!comport->mReadFlag)
            comport->sleep(10);
       return comport;
    }
    return 0;
}

SerialPort::~SerialPort() {
    mTreadFlag = false;
    close(mCom);
    pthread_join((pthread_t)mThread, 0);
}

unsigned int SerialPort::write_native(const void *data, unsigned int len) {
	ssize_t bw;
	size_t total = 0;
	do {
		bw = write(mCom, (char *)data + total, len - total);
		total += bw;
		if(bw)
			tcdrain(mCom);
	} while(bw > 0 && total < len);
	return total;
}

void SerialPort::sleep(unsigned int ms) {
	usleep(ms*1000);
}

#if( defined(__APPLE__) || defined(__MACH__) )
const static char TTYUSB_PATTERN[] = "tty.";
#else
const static char TTYUSB_PATTERN[] = "ttyUSB";
#endif

const char *SerialPort::findNextPort(bool finish) {
	static char buf[512];
	static DIR *dp = 0;
	struct dirent *entry;
	buf[0] = 0;
	if(!dp) {
		dp = opendir("/dev");
		if(!dp)
			return buf;
	}
	if(dp) {
		while((entry = readdir(dp)) != NULL) {
			if(strncmp(entry->d_name, TTYUSB_PATTERN, sizeof(TTYUSB_PATTERN) - 1) == 0) {
				snprintf(buf, sizeof(buf), "%s/%s", "/dev", entry->d_name);
				break;
			}
		}
	}
	if(finish) {
		if(dp) {
			closedir(dp);
			dp = 0;
		}
	}
	return buf;
}

unsigned int SerialPort::getTick() {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec / 1000UL + (tv.tv_sec % 3600000UL) * 1000UL;
}

void SerialPort::setRts(bool val) {
    int flag;
    if(ioctl(mCom, TIOCMGET, &flag) != -1) {
		if(val)
			flag |= TIOCM_RTS;
		else
			flag &= ~TIOCM_RTS;
		ioctl(mCom, TIOCMSET, &flag);
    }
}

void SerialPort::setDtr(bool val) {
    int flag;
    if(ioctl(mCom, TIOCMGET, &flag) != -1) {
		if(val)
			flag |= TIOCM_DTR;
		else
			flag &= ~TIOCM_DTR;
		ioctl(mCom, TIOCMSET, &flag);
    }
}

#endif
