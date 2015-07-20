/*
 * serialport.h
 *
 * Cross-platform serial port implementation
 *
 *  Created on: 2014
 *      Author: Nikolay Khabarov
 *  License: You can do whatever you want. Author doesn't provide warranty of any kind.
 *
 */

#ifndef SERIALPORT_H
#define SERIALPORT_H
#include <stdio.h>

#if ( defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) )
#define COMWINDOWS
typedef void *HANDLE;
#define COM HANDLE
typedef unsigned long DWORD;
typedef void *PVOID,*LPVOID;
#else
#define COMPOSIX
#define COM int
#endif

#define ERROR_WRITE_STRING (char*)"ERROR ---> CAN'T WRITE TO PORT\n"
#define ERROR_READ_STRING (char*)"ERROR ---> CAN'T READ FROM PORT\n"

class SerialPort
{
public:
    static SerialPort *open(const char *port);
    virtual ~SerialPort();
    void send(const char *text);
    void send(char c);
    void send(const void *data, unsigned int len);
    bool waitAnswer(unsigned int len, unsigned int timeOutMs);
    bool isReadError();
    static const char *findNextPort(bool finish);
private:
#ifdef COMWINDOWS
    static DWORD ThreadProc (LPVOID lpdwThreadParam);
#else
    static void * thread_start(void *arg);
#endif
    SerialPort(COM comport);
    unsigned int write_native(const void *data, unsigned int len);
    bool read_native(const void *data, unsigned int len, unsigned int *rb);
    void sleep(unsigned int ms);
    COM get_com();
    COM mCom;
    bool mTreadFlag;
    void *mThread;
    unsigned int mBytesRecivedSinceLastSend;
    bool mReadError;
};

extern void SerialPortRecieved(SerialPort *port, const char *text, unsigned int len);
extern void SerialPortError(SerialPort *port, const char *text);

#endif // SERIALPORT_H
