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

#ifdef TERMINALWINDOWS
#include <conio.h>
#include <stdio.h>
#include <stdarg.h>
#include <windef.h>
#include <strings.h>
#include <winbase.h>
#include <wincon.h>

void moveCursor(short x, short y) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	if(!GetConsoleScreenBufferInfo(handle, &info))
		return;
	info.dwCursorPosition.X += x;
	info.dwCursorPosition.Y += y;
	SetConsoleCursorPosition(handle, info.dwCursorPosition);
}

void moveInputLine(short s, short d) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	if(!GetConsoleScreenBufferInfo(handle, &info))
		return;
	SMALL_RECT sr;
	COORD nr;
	sr.Left = info.dwCursorPosition.X + s;
	sr.Right = info.dwSize.X;
	nr.Y =  sr.Top = sr.Bottom = info.dwCursorPosition.Y;
	nr.X = sr.Left + d;
	CHAR_INFO ci;
	ci.Char.UnicodeChar = 0;
	ci.Char.AsciiChar = ' ';
	ci.Attributes = info.wAttributes;
	ScrollConsoleScreenBuffer(handle, &sr, NULL, nr, &ci);
}

void eraseLine() {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	if(!GetConsoleScreenBufferInfo(handle, &info))
		return;
	COORD tc;
	DWORD bw;
	tc.X = 0;
	tc.Y = info.dwCursorPosition.Y;
	FillConsoleOutputCharacter(handle, ' ', info.dwSize.X, tc, &bw);
	moveCursor(-info.dwCursorPosition.X, 0);
}

void Terminal::put(char c) {
	static bool escRecieving = false;
	static unsigned int escSequencePos = 0;
	static char escSequence[5] = {0};
	if(c == 0x1B) { // ESC character
		escRecieving = true;
	} else if(escRecieving) {
		escSequence[escSequencePos++] = c;
		escSequence[escSequencePos] = 0;
		if(escSequencePos == 1 && c == '[')
			return;
		if((c >= 0x40 && c <= 0x7e) || escSequencePos >= sizeof(escSequence) - 1) {
			if(strcmp(escSequence, "[C") == 0) { // Right
				moveCursor(1, 0);
			} else if(strcmp(escSequence, "[D") == 0) { // Left
				moveCursor(-1, 0);
			} else if(strcmp(escSequence, "[@") == 0) { // Left
				moveInputLine(0, 1);
			} else if(strcmp(escSequence, "[1P") == 0) { // Del
				moveInputLine(1, -1);
			} else if(strcmp(escSequence, "[K") == 0) { // Erase Line
				eraseLine();
			}
			escRecieving = false;
			escSequencePos = 0;
		}
	}else {
		putchar(c);
	}
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

}

Terminal::~Terminal() {

}

void Terminal::get(char *buf) {
	bool repeat = false;
	do {
		char *bp = buf;
		int c =  _getch();
		if(c < 0) {
			repeat = true;
		} else if(c == 0xe0) {
			*bp++ = 0x1b;
			*bp++ = '[';
			switch(_getch()) {
			case 'K':
				*bp++ = 'D';
				break;
			case 'M':
				*bp++ = 'C';
				break;
			case 'H':
				*bp++ = 'A';
				break;
			case 'P':
				*bp++ = 'B';
				break;
			case 'S':
				*bp++ = '3';
				*bp++ = '~';
				break;
			default:
				repeat = true;
			}
		} else {
			*bp++ = c;
		}
		*bp = 0;
	} while(repeat);
}


#endif // TERMINALWINDOWS
