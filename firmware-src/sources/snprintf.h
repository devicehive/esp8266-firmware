/**
 *	\file		snprintf.h
 *	\brief		Tiny snprintf implementation.
 *	\author		Nikolay Khabarov
 *	\date		2014
 *	\copyright	Public Domain
 */

#ifndef SNPRINTF_H_
#define SNPRINTF_H_
#include <stdio.h>
#include <stdarg.h>

/**
 *	\brief				Write formatted data to char array.
 *	\details			Lightweight version of snprintf for embedded systems.
 *	\param[out]	pString	Output buffer.
 *	\param[in]	length	Output buffer maximum size(including space for null terminated char).
 *	\param[in]	pFormat	Format.
 *	\param[in]	...		Additional variables, specified by format.
 *	\return				Number of character that was printed in buffer(excluding null terminated char).
 */
int snprintf(char *pString, size_t length, const char *pFormat, ...);

/**
 *	\brief				Write formatted data to char array.
 *	\details			Lightweight version of snprintf for embedded systems.
 *	\param[out]	pString	Output buffer.
 *	\param[in]	length	Output buffer maximum size(including space for null terminated char).
 * 	\param[in]	pFormat	Format.
 *	\param[in]	ap		Additional variables, specified by format.
 *	\return				Number of character that was printed in buffer(excluding null terminated char).
 */
int vsnprintf(char *pString, size_t length, const char *pFormat, va_list ap);

#endif /* SNPRINTF_H_ */
