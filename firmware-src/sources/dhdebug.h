/**
 *	\file		dhdebug.h
 *	\brief		Print debug text.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHDEBUG_H_
#define _DHDEBUG_H_

/**
 *	\brief			Switch debug output to dhterminal
 */
void dhdebug_terminal();

/**
 *	\brief			Switch debug output to UART
 */
void dhdebug_direct();

/**
 *	\brief			Print to debug output.
 *	\param[in]	fmt	Format.
 *	\param[in]	...	Additional arguments specified by format
 */
void dhdebug(const char *fmt, ...);

/**
 *	\brief				Print dumped data in hex.
 *	\param[in]	data	Data for dump.
 *	\param[in]	len		Data size in bytes.
 */
void dhdebug_dump(const char *data, unsigned int len);

#endif /* _DHDEBUG_H_ */
