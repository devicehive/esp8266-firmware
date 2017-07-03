/*
 *	\file		dhconnector_websocket.h
 *	\brief		Main connectivity to DeviceHive with WebSocket
 *	\author		Nikolay Khabarov
 *	\date		2017
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHCONNECTOR_WEBSOCKET_H_
#define _DHCONNECTOR_WEBSOCKET_H_

/** Function prototype for sending data. */
typedef int (*dhconnector_websocket_send_proto)(const char *data, unsigned int len);
/** Function prototype for error callback. */
typedef void (*dhconnector_websocket_error)(void);

/**
 *	\brief					Initialize devicehive WebSocket protocol exchange
 *	\param[in]	send_func	Pointer to function to call to send data.
 *	\param[in]	err_func	Pointer to function to call on error.
 */
void dhconnector_websocket_start(dhconnector_websocket_send_proto send_func, dhconnector_websocket_error err_func);

/**
 *	\brief					Stop any protocol activities.
 */
void dhconnector_websocket_stop();

/**
 *	\brief					Parse received from server data.
 *	\param[in]	data		Pointer to data.
 *	\param[in]	len			Number of bytes in data.
 */
void dhconnector_websocket_parse(const char *data, unsigned int len);

#endif /* _DHCONNECTOR_WEBSOCKET_H_ */
