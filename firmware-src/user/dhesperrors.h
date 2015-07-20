/*
 * dhesperrors.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef USER_DHESPERRORS_H_
#define USER_DHESPERRORS_H_

void dhesperrors_disconnect_reason(const char *descrption, uint8 reason) ;
void dhesperrors_espconn_result(const char *descrption, int reason);

#endif /* USER_DHESPERRORS_H_ */
