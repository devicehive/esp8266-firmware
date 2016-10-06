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

#include "c_types.h" // ICACHE_FLASH_ATTR

int ICACHE_FLASH_ATTR vsnprintf(char *pString, size_t length, const char *pFormat, va_list ap)
{
	char digitBuffer[32];
	char *pOriginalStr = pString;
	length--; // for null terminated char

	/* Phase string */
	while (*pFormat != 0 && (pString-pOriginalStr) < length){
		if (*pFormat != '%'){
			*pString++ = *pFormat++;
		} else if (*(pFormat+1) == '%'){
			*pString++ = '%';
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
					*pString++ = '-';
				}
				do {
					*tmp++ = val % 10 + '0';
				} while ( (val /= 10) > 0);
				tmp--;
				while( (pString-pOriginalStr) < length && tmp >= digitBuffer) *pString++ = *tmp--;
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
				while( (pString-pOriginalStr) < length && tmp >= digitBuffer) *pString++ = *tmp--;
			}
			break;
			case 's': {
				char * tmp = va_arg(ap, char *);
				while (*tmp && (pString-pOriginalStr) < length) *pString++ = *tmp++;
				break;
			}
			case 'c': *pString++ = va_arg(ap, unsigned int); break;
			case 'f':{ // .4f
				char *tmp = digitBuffer;
				double z = va_arg(ap, double);
				long val;
				// TODO val can be overloaded
				if(z >= 0.0f) {
					val = (z*10000.0f + 0.5f);
				} else {
					val = (-z*10000.0f + 0.5f);
					*pString++ = '-';
				}
				int i = 0;
				do {
					*tmp++ = val % 10 + '0';
					i++;
					if(i == 4)
						*tmp++ = '.';
				} while ( (val /= 10) > 0 || i < 5);
				tmp--;
				while( (pString-pOriginalStr) < length && tmp >= digitBuffer) *pString++ = *tmp--;
			}
			break;
			default:
				return EOF;
			}

			pFormat++;
		}
	}

	*pString = 0;

	return (pString-pOriginalStr);
}

int ICACHE_FLASH_ATTR snprintf(char *pString, size_t length, const char *pFormat, ...)
{
    va_list    ap;
    int rc;

    va_start(ap, pFormat);
    rc = vsnprintf(pString, length, pFormat, ap);
    va_end(ap);

    return rc;
}
