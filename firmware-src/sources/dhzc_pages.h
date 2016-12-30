/**
 *	\file		dhzc_pages.h
 *	\brief		HTML pages for zero configuration.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHZC_PAGES_H_
#define _DHZC_PAGES_H_

/**
 *	\brief				Get page HTML with error.
 *	\param[out]	error	Pointer to string with error description
 *	\param[out]	len		Pointer to page size in bytes.
 *	\return 			Pointer to page or zero if there is not enough memory.
 */
const char * dhzc_pages_error(const char *error, unsigned int *len);

/**
 *	\brief				Get page HTML with data accepted result.
 *	\param[out]	len		Pointer to page size in bytes.
 *	\return 			Pointer to page or zero if there is not enough memory.
 */
const char * dhzc_pages_ok(unsigned int *len);

/**
 *	\brief				Get page HTML with configuration form.
 *	\param[out]	size	Pointer to page size in bytes.
 *	\return 			Pointer to page or zero if there is not enough memory.
 */
const char * dhzc_pages_form(unsigned int *len);

#endif /* _DHZC_PAGES_H_ */
