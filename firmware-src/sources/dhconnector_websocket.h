/*
 *	\file		dhconnector_websocket.h
 *	\brief		Main connectivity to DeviceHive with WebSocket
 *	\author		Nikolay Khabarov
 *	\date		2017
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHCONNECTOR_WEBSOCKET_H_
#define _DHCONNECTOR_WEBSOCKET_H_

/** Function prototype for sendind data. */
typedef void (*dhconnector_websocket_send_proto)(const char *data, unsigned int len);

void dhconnector_websocket_start(dhconnector_websocket_send_proto send_func);
void dhconnector_websocket_parse(const char *data, unsigned int len);

#endif /* _DHCONNECTOR_WEBSOCKET_H_ */
