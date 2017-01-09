/**
 *	\file		dhap.h
 *	\brief		Wireless firmware configuration util
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHAP_H_
#define _DHAP_H_

/**
 *	\brief					Initialize access point.
 *	\param[in]	ssid		Wireless network name.
 *	\param[in]	password	Wireless network password. Can be NULL or empty string for open network.
 */
void dhap_init(const char *ssid, const char *password);

/**
 *	\brief		Return AP ipinfo.
 *	\return 	Pointer to ip_info struct.
 */
const struct ip_info * dhap_get_ip_info();

#endif /* SOURCES_DHAP_H_ */
