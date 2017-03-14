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

int dhconnector_websocket_api_start(char *buf, unsigned int maxlen);
int dhconnector_websocket_api_communicate(const char *in, unsigned int inlen, char *out, unsigned int outmaxlen);

#endif /* _DHCONNECTOR_WEBSOCKET_API_H_ */
