/*
 * main.cpp
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: simple VT100 terminal implementation
 *
 */

#ifndef TERMINAL_H_
#define TERMINAL_H_

#if ( defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) )
#define TERMINALWINDOWS
#else
#define TERMINALPOSIX
#endif

class Terminal {
public:
	static Terminal *open();
	virtual ~Terminal();
	void get(char *buf);
	void put(char c);
	void put(const char *str);
private:
	Terminal();
};

#endif /* TERMINAL_H_ */
