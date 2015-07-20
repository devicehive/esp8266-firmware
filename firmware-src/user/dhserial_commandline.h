/*
 * dhserial_commandline.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef USER_DHSERIAL_COMMANDLINE_H_
#define USER_DHSERIAL_COMMANDLINE_H_

void dhserial_commandline_do(const char *command);
char *dhserial_commandline_autocompleater(const char *pattern);

#endif /* USER_DHSERIAL_COMMANDLINE_H_ */
