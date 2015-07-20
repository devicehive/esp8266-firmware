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

#include "terminal.h"

#ifdef TERMINALPOSIX
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>

static struct termios mOldStdinAttr;

void Terminal::put(char c) {
    putchar(c);
    fflush( stdout );
}

void Terminal::put(const char *str) {
    while(*str)
    	put(*str++);
}

Terminal *Terminal::open() {
	static Terminal *term = 0;
	if(!term)
		term = new Terminal();
	return term;
}

Terminal::Terminal() {
	memset(&mOldStdinAttr, 0, sizeof(mOldStdinAttr));
	tcgetattr(STDIN_FILENO, &mOldStdinAttr);
	struct termios stdinattr;
	tcgetattr(STDIN_FILENO, &stdinattr);
	stdinattr.c_lflag &= ~ICANON;
	stdinattr.c_lflag &= ~ECHO;
	stdinattr.c_cc[VMIN] = 1;
	stdinattr.c_cc[VTIME] = 0;
	stdinattr.c_cc[VEOF]  = 3;
	stdinattr.c_cc[VINTR] = 0;
	stdinattr.c_cc[VSTART] = 0;
	tcsetattr(0, TCSANOW, &stdinattr);
	tcsetattr(STDIN_FILENO, TCSANOW, &stdinattr);
}

Terminal::~Terminal() {
	tcsetattr(STDIN_FILENO, TCSANOW, &mOldStdinAttr);
}

void Terminal::get(char *buf) {
	if(read(STDIN_FILENO, buf, 1))
		buf[1] = 0;
	else
		buf[0] = 0;
}

#endif // TERMINALPOSIX
