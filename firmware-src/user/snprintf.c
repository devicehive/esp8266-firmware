/*
 * snprintf.c
 *
 * Tiny snprintf() implementation for embedded system
 *
 *  Created on: 2014
 *      Author: Nikolay Khabarov
 *  License: You can do whatever you want. Author doesn't provide warranty of any kind.
 *
 */

#include "snprintf.h"

int vsnprintf(char *pStr, size_t length, const char *pFormat, va_list ap)
{
	char digitBuffer[32];
	char *pOriginalStr = pStr;
	length--; // for null terminated char

	/* Phase string */
	while (*pFormat != 0 && (pStr-pOriginalStr) < length){
		if (*pFormat != '%'){
			*pStr++ = *pFormat++;
		} else if (*(pFormat+1) == '%'){
			*pStr++ = '%';
			pFormat += 2;
		} else {
			pFormat++;

			switch (*pFormat) {
			case 'd':
			case 'i':{
				char *tmp = digitBuffer;
				long val = va_arg(ap, signed long);
				if(val < 0) {
					val = -val;
					*pStr++ = '-';
				}
				do {
					*tmp++ = val % 10 + '0';
				} while ( (val /= 10) > 0);
				tmp--;
				while( (pStr-pOriginalStr) < length && tmp >= digitBuffer) *pStr++ = *tmp--;
			}
			break;
			case 'u':
			case 'x':
			case 'X':
			{
				unsigned int base = (*pFormat == 'u') ? 10 : 16;
				char *tmp = digitBuffer;
				unsigned long val = va_arg(ap, unsigned long);
				do {
					unsigned int c = val % base;
					if(c < 10)
						*tmp++ = c + '0';
					else if(*pFormat == 'x')
						*tmp++ = c - 10 + 'a';
					else
						*tmp++ = c - 10 + 'A';
				} while ( (val /= base) > 0);
				tmp--;
				while( (pStr-pOriginalStr) < length && tmp >= digitBuffer) *pStr++ = *tmp--;
			}
			break;
			case 's': {
				char * tmp = va_arg(ap, char *);
				while (*tmp && (pStr-pOriginalStr) < length) *pStr++ = *tmp++;
				break;
			}
			case 'c': *pStr++ = va_arg(ap, unsigned int); break;
			case 'f':{ // .4f
				char *tmp = digitBuffer;
				double z = va_arg(ap, double);
				long val;
				// TODO val may be overloaded
				if(z >= 0.0f) {
					val = (z*10000.0f + 0.5f);
				} else {
					val = (-z*10000.0f + 0.5f);
					*pStr++ = '-';
				}
				int i = 0;
				do {
					*tmp++ = val % 10 + '0';
					i++;
					if(i == 4)
						*tmp++ = '.';
				} while ( (val /= 10) > 0 || i < 5);
				tmp--;
				while( (pStr-pOriginalStr) < length && tmp >= digitBuffer) *pStr++ = *tmp--;
			}
			break;
			default:
				return EOF;
			}

			pFormat++;
		}
	}

	*pStr = 0;

	return (pStr-pOriginalStr);
}

int snprintf(char *pString, size_t length, const char *pFormat, ...)
{
    va_list    ap;
    int rc;

    va_start(ap, pFormat);
    rc = vsnprintf(pString, length, pFormat, ap);
    va_end(ap);

    return rc;
}
