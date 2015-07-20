/*
 * dhsettings.h
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#ifndef USER_DHSETTINGS_H_
#define USER_DHSETTINGS_H_

#define DHSETTINGS_SERVER_MAX_LENGTH 384
#define DHSETTINGS_DEVICEID_MAX_LENGTH 128
#define DHSETTINGS_DEVICEKEY_MAX_LENGTH 128

int dhsettings_init();
int dhsettings_commit();
int dhsettings_clear();

const char *dhsettings_get_wifi_ssid();
const char *dhsettings_get_wifi_password();
const char *dhsettings_get_devicehive_server();
const char *dhsettings_get_devicehive_deviceid();
const char *dhsettings_get_devicehive_devicekey();
void dhsettings_set_wifi_ssid(const char *ssid);
void dhsettings_set_wifi_password(const char *pass);
void dhsettings_set_devicehive_server(const char *server);
void dhsettings_set_devicehive_deviceid(const char *id);
void dhsettings_set_devicehive_devicekey(const char *key);

#endif /* USER_DHSETTINGS_H_ */
