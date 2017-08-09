/*
 *	\file		dhconnector_websocket_api.h
 *	\brief		Main connectivity to DeviceHive via WebSocket
 *	\author		Nikolay Khabarov
 *	\date		2017
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHCONNECTOR_WEBSOCKET_API_H_
#define _DHCONNECTOR_WEBSOCKET_API_H_

#define DHCONNECT_WEBSOCKET_API_ERROR -1

/**
 *	\brief					Initialize communication with DeviceHive server
 *	\param[out]	data		Pointer to store data which should be sent to server.
 *	\param[in]	maxlen		Maximum size of specified buffer in bytes.
 *	\return 				Number of copied bytes.
 */
int dhconnector_websocket_api_start(char *buf, unsigned int maxlen);

/**
 *	\brief					Exchange data with server.
 *	\param[in]	in			Data received from server.
 *	\param[in]	inlen		Number of bytes received from server.
 *	\param[in]	out			Pointer to store data which should be sent to server.
 *	\param[in]	outmaxlen	Maximum size of specified buffer in bytes.
 *	\return 				Number of copied bytes or DHCONNECT_WEBSOCKET_API_ERROR on error.
 */
int dhconnector_websocket_api_communicate(const char *in, unsigned int inlen, char *out, unsigned int outmaxlen);

/**
 *	\brief					Check if connection is established and command subscription is successfully set.
 *	\return 				Non zero value if successfully connected and zero otherwise.
 */
int dhconnector_websocket_api_check();

#endif /* _DHCONNECTOR_WEBSOCKET_API_H_ */
