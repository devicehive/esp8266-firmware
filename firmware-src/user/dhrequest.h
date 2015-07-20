/*
 * dhrequest.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef USER_DHREQUEST_H_
#define USER_DHREQUEST_H_

typedef struct __attribute__((packed)) HTTP_REQUEST HTTP_REQUEST;
struct __attribute__((packed)) HTTP_REQUEST{
	HTTP_REQUEST *next;
	unsigned int len;
	char body[];
};

void dhrequest_load_settings();
const char *dhrequest_current_server();
HTTP_REQUEST *dhrequest_create_register();
HTTP_REQUEST *dhrequest_create_poll(const char *timestamp);
HTTP_REQUEST *dhrequest_create_update(int commandId, const char *status, const char *result);
HTTP_REQUEST *dhrequest_create_info();
HTTP_REQUEST *dhrequest_create_notification(const char *name, const char *parameters);

HTTP_REQUEST *dhrequest_update_poll(HTTP_REQUEST *old, const char *timestamp);

void dhrequest_free(HTTP_REQUEST *request);

const char *dhrequest_find_http_responce_code(const char *data, unsigned short len);

#endif /* USER_DHREQUEST_H_ */
