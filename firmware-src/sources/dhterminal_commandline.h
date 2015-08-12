/**
 *	\file		dhterminal_commandline.h
 *	\brief		Commands interpreter for terminal.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef USER_DHTERMINAL_COMMANDLINE_H_
#define USER_DHTERMINAL_COMMANDLINE_H_

/**
 *	\brief				Perform command from terminal
 *	\param[in]	command	String with command.
 */
void dhterminal_commandline_do(const char *command);

/**
 *	\brief				Terminal commands autocompleter.
 *	\details			Functions remember previous return value and next call will happen with the same value, function will continue search with original pattern.
 *	\param[in]	pattern	Beginning of command.
 *	\return 			String with matching command or zero if no commands found.
 */
char *dhterminal_commandline_autocompleter(const char *pattern);

#endif /* USER_DHTERMINAL_COMMANDLINE_H_ */
