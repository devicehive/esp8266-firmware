/**
 *	\file		mdnsd.h
 *	\brief		Simple mDNS implementation
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */

#ifndef _MDNS_H_
#define _MDNS_H_

/**
 *	\brief				Initialize mDNS daemon.
 *	\details			Must be called after getting IP address. 1st level domain is always 'local'.
 *						Max length is 64 bytes including zero terminated char.
 *	\param[in]	name	Pointer to static string with 2nd level domain name.
 *	\param[in]	addr	Corresponding IP address.
 *	\return				Zero on error, non-zero on success.
 */
int mdnsd_start(const char *name, unsigned long addr);

/**
 *	\brief				Stops mDNS daemon.
 */
void mdnsd_stop(void);

#endif /* _MDNS_H_ */
