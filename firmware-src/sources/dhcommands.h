/**
 *	\file		dhcommands.h
 *	\brief		Handle commands from remote server.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHCOMMANDS_H_
#define _DHCOMMANDS_H_

#include "dhsender_data.h"

/**
 *	\brief					Handle remote command.
 *	\param[in]	cb			Callback descriptor for result.
 *	\param[in]	command		Null terminated string with command.
 *	\param[in]	params		Pointer to JSON with parameters.
 *	\param[in]	paramslen	JSON parameters length in bytes.
 */
void dhcommands_do(COMMAND_RESULT *cb, const char *command, const char *params,
		unsigned int paramslen);

#endif /* _DHCOMMANDS_H_ */
