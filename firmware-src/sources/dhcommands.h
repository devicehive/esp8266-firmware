/**
 *	\file		dhcommands.h
 *	\brief		Handle commands from remote server.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef USER_DHCOMMANDS_H_
#define USER_DHCOMMANDS_H_

/**
 *	\brief					Handle remote command.
 *	\param[in]	id			Command id.
 *	\param[in]	command		Null terminated string with command.
 *	\param[in]	params		Pointer to JSON with parameters.
 *	\param[in]	paramslen	JSON parameters length in bytes.
 */
void dhcommands_do(int id, const char *command, const char *params, unsigned int paramslen);

#endif /* USER_DHCOMMANDS_H_ */
