/**
 *	\file		dhconnector.h
 *	\brief		Main connectivity to DeviceHive server for receiving commands.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHCONNECTOR_H_
#define _DHCONNECTOR_H_

/** Function prototype for handling function. */
typedef void (*dhconnector_command_json_cb)(unsigned int id, const char *command, const char *params, unsigned int paramslen);

/** Current connection state. */
typedef enum {
	CS_DISCONNECT,	///< Disconnected from DeviceHive server.
	CS_GETINFO,		///< Getting info from DeviceHive server.
	CS_REGISTER,	///< Registering on DeviceHive server.
	CS_POLL,		///< Polling command from DeviceHive server.
} CONNECTION_STATE;

/**
 *	\brief			Initializes connector, parameters are taken from permanent storage.
 *	\param[in]	cb	Pointer to command callback.
 */
void dhconnector_init(dhconnector_command_json_cb cb);

/**
 *	\brief			Get connector current state.
 *	\return			Current state value from CONNECTION_STATE enum.
 */
CONNECTION_STATE dhconnector_get_state();

#endif /* _DHCONNECTOR_H_ */
