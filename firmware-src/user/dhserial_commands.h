/*
 * dhserial_commands.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef USER_DHSERIAL_COMMANDS_H_
#define USER_DHSERIAL_COMMANDS_H_

int dhserial_commands_is_busy();

void dhserial_commands_uname(const char *args);
void dhserial_commands_status(const char *args);
void dhserial_commands_scan(const char *args);
void dhserial_commands_debug(const char *args);
void dhserial_commands_history(const char *args);
void dhserial_commands_reboot(const char *args);
void dhserial_commands_config(const char *args);
void dhserial_commands_configure(const char *args);
void dhserial_commands_echo(const char *args);
void dhserial_commands_nslookup(const char *args);
void dhserial_commands_ping(const char *args);

#endif /* USER_DHSERIAL_COMMANDS_H_ */
