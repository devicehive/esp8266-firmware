/**
 *	\file		dhterminal_commands.h
 *	\brief		Command for terminal
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHTERMINAL_COMMANDS_H_
#define _DHTERMINAL_COMMANDS_H_

/**
 *	\brief		Check if command is currently working.
 *	\return 	Non zero value if command is working or zero value if nothing in progress.
 */
int dhterminal_commands_is_busy(void);

/**
 *	\brief				Print firmware version.
 *	\param[in]	args	Arguments for command. Ignoring for this command, just to fit prototype.
 */
void dhterminal_commands_uname(const char *args);

/**
 *	\brief				Print current system status.
 *	\param[in]	args	Arguments for command. Ignoring for this command, just to fit prototype.
 */
void dhterminal_commands_status(const char *args);

/**
 *	\brief				Scan air for wireless networks.
 *	\param[in]	args	Arguments for command. Ignoring for this command, just to fit prototype.
 */
void dhterminal_commands_scan(const char *args);

/**
 *	\brief				Enable debug output.
 *	\param[in]	args	Arguments for command. Ignoring for this command, just to fit prototype.
 */
void dhterminal_commands_debug(const char *args);

/**
 *	\brief				Print history.
 *	\param[in]	args	Arguments for command. Ignoring for this command, just to fit prototype.
 */
void dhterminal_commands_history(const char *args);

/**
 *	\brief				Reboot device.
 *	\param[in]	args	Arguments for command. Ignoring for this command, just to fit prototype.
 */
void dhterminal_commands_reboot(const char *args);

/**
 *	\brief				Print current config.
 *	\param[in]	args	Arguments for command. Ignoring for this command, just to fit prototype.
 */
void dhterminal_commands_config(const char *args);

/**
 *	\brief				Runs configure util.
 *	\param[in]	args	Arguments for command.
 */
void dhterminal_commands_configure(const char *args);

/**
 *	\brief				Print back the same.
 *	\param[in]	args	Text.
 */
void dhterminal_commands_echo(const char *args);

/**
 *	\brief				Resolve domain address.
 *	\param[in]	args	String with domain address.
 */
void dhterminal_commands_nslookup(const char *args);

/**
 *	\brief				Ping remote host.
 *	\param[in]	args	String with domain address or IP address.
 */
void dhterminal_commands_ping(const char *args);

#endif /* _DHTERMINAL_COMMANDS_H_ */
