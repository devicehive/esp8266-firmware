/*
 * snprintf.h
 *
 *  Created on: 2014
 *      Author: Nikolay Khabarov
 */

#ifndef SNPRINTF_H_
#define SNPRINTF_H_
#include <stdio.h>
#include <stdarg.h>

int snprintf(char *pString, size_t length, const char *pFormat, ...);
int vsnprintf(char *pStr, size_t length, const char *pFormat, va_list ap);

#endif /* SNPRINTF_H_ */
