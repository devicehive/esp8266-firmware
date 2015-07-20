/*
 * dhconnector.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef USER_DHCONNECTOR_H_
#define USER_DHCONNECTOR_H_

typedef void (*dhconnector_command_json_cb)(int id, const char *command, const char *params, unsigned int paramslen);

typedef enum {
	CS_DISCONNECT,
	CS_GETINFO,
	CS_REGISTER,
	CS_POLL,
} CONNECTION_STATE;

void dhconnector_init(dhconnector_command_json_cb cb);
CONNECTION_STATE dhconnector_get_state();

#endif /* USER_DHCONNECTOR_H_ */
