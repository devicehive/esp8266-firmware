/*
 * dhcommand.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef USER_DHCOMMAND_H_
#define USER_DHCOMMAND_H_

void dhcommand_do(int id, const char *command, const char *params, unsigned int paramslen);

#endif /* USER_DHCOMMAND_H_ */
