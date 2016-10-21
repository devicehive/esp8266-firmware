/**
 *	\file		dhsettings.h
 *	\brief		Permanent data storage for firmware.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHSETTINGS_H_
#define _DHSETTINGS_H_

/** Wi-Fi SSID string max length. */
#define DHSETTINGS_SSID_MAX_LENGTH 32
/** Wi-Fi passwork string max length. */
#define DHSETTINGS_PASSWORD_MAX_LENGTH 64
/** DeviceHive server string max length. */
#define DHSETTINGS_SERVER_MAX_LENGTH 384
/** DeviceID string max length. */
#define DHSETTINGS_DEVICEID_MAX_LENGTH 128
/** AccessKey string max length. */
#define DHSETTINGS_ACCESSKEY_MAX_LENGTH 65

/**
 *	\brief				Inits settings, reads values from storage.
 *	\param[out]	saved	Pointer to store confirm that settings were never saved
 *						before. If flash was successfully read and setting were
 *						never saved, zero will be stored. Non zero otherwise.
 *	\return 			Non zero value on success. Zero on error.
 */
int dhsettings_init(int *saved);

/**
 *	\brief			Saves values to permanent storage.
 *	\return 		Non zero value on success. Zero on error.
 */
int dhsettings_commit();

/**
 *	\brief			Destroy data in permanent storage.
 *	\return 		Non zero value on success. Zero on error.
 */
int dhsettings_clear();

/**
 *	\brief			Get Wi-Fi SSID.
 *	\return 		Pointer to buffer with null terminated string.
 */
const char *dhsettings_get_wifi_ssid();

/**
 *	\brief			Get Wi-Fi password.
 *	\return 		Pointer to buffer with null terminated string.
 */
const char *dhsettings_get_wifi_password();

/**
 *	\brief			Get DeviceHive server.
 *	\return 		Pointer to buffer with null terminated string.
 */
const char *dhsettings_get_devicehive_server();

/**
 *	\brief			Get DeviceHive DeviceId.
 *	\return 		Pointer to buffer with null terminated string.
 */
const char *dhsettings_get_devicehive_deviceid();

/**
 *	\brief			Get DeviceHive AccessKey.
 *	\return 		Pointer to buffer with null terminated string.
 */
const char *dhsettings_get_devicehive_acceykey();

/**
 *	\brief				Set Wi-Fi SSID.
 *	\param[in]	ssid	Pointer to buffer with null terminated string.
 */
void dhsettings_set_wifi_ssid(const char *ssid);
/**
 *	\brief				Set Wi-Fi password.
 *	\param[in]	pass	Pointer to buffer with null terminated string.
 */
void dhsettings_set_wifi_password(const char *pass);

/**
 *	\brief				Set DeviceHive server.
 *	\param[in]	server	Pointer to buffer with null terminated string.
 */
void dhsettings_set_devicehive_server(const char *server);

/**
 *	\brief				Set DeviceHive DeviceId.
 *	\param[in]	id		Pointer to buffer with null terminated string.
 */
void dhsettings_set_devicehive_deviceid(const char *id);

/**
 *	\brief				Set DeviceHive AccessKey.
 *	\param[in]	key		Pointer to buffer with null terminated string.
 */
void dhsettings_set_devicehive_accesskey(const char *key);

/**
 *	\brief			Check if char is suitable for DeviceId string
 *	\param[in]	c	Char for test.
 *	\return			Non zero value if char is suitable, zero otherwise.
 */
int dhsettings_deviceid_filter(char c);

/**
 *	\brief			Check if char is suitable for AccessKey string
 *	\param[in]	c	Char for test.
 *	\return			Non zero value if char is suitable, zero otherwise.
 */
int dhsettings_accesskey_filter(char c);

/**
 *	\brief			Check if char is suitable for API Url string
 *	\param[in]	c	Char for test.
 *	\return			Non zero value if char is suitable, zero otherwise.
 */
int dhsettings_server_filter(char c);

#endif /* _DHSETTINGS_H_ */
