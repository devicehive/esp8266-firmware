/*
 * dhsender.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef DHSENDER_H_
#define DHSENDER_H_

#include <ip_addr.h>

static const char STATUS_OK[] = "OK";
static const char STATUS_ERROR[] = "Error";

void dhsender_init(ip_addr_t *ip, int port);
void dhsender_response(int id, const char *status, const char *result);

#endif /* DHSENDER_H_ */
