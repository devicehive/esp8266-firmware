/**
 *	\file		dhcommands.h
 *	\brief		Handle commands from remote server.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHCOMMANDS_H_
#define _DHCOMMANDS_H_

/**
 *	\brief					Handle remote command.
 *	\param[in]	id			Command id.
 *	\param[in]	command		Null terminated string with command.
 *	\param[in]	params		Pointer to JSON with parameters.
 *	\param[in]	paramslen	JSON parameters length in bytes.
 */
void dhcommands_do(unsigned int id, const char *command, const char *params, unsigned int paramslen);

#endif /* _DHCOMMANDS_H_ */
