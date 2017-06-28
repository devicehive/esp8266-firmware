/**
 *	\file		dhconnector.h
 *	\brief		Main connectivity to DeviceHive server for receiving commands.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHCONNECTOR_H_
#define _DHCONNECTOR_H_

#include "dhrequest.h"
#include "dhsender_data.h"

/** Current connection state. */
typedef enum {
	CS_DISCONNECT,		///< Disconnected from DeviceHive server.
	CS_RESOLVEHTTP,		///< Resolve HTTP API url.
	CS_GETINFO,			///< Getting info from DeviceHive server with WebSocket url.
	CS_RESOLVEWEBSOCKET,///< Resolve WebSocket url.
	CS_WEBSOCKET,		///< Request to switch protocol to WebSocket.
	CS_OPERATE			///< Normal operational state.
} CONNECTION_STATE;

/**
 *	\brief			Initializes connector, parameters are taken from permanent storage.
 */
void dhconnector_init(void);

/**
 *	\brief			Get connector current state.
 *	\return			Current state value from CONNECTION_STATE enum.
 */
CONNECTION_STATE dhconnector_get_state(void);

/**
 *	\brief			Callback for custom firmware. If callback returns non null request, it will be sent instead poll request.
 *	\return			HTTP request for DH server or null to keep normal mode.
 */
HTTP_REQUEST *custom_firmware_request(void);

#endif /* _DHCONNECTOR_H_ */
