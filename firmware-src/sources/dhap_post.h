/**
 *	\file		dhap_post.h
 *	\brief		POST data parser.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _DHAP_POST_H_
#define _DHAP_POST_H_

/**
 *	\brief				Parse and appy POST data from HTTP request.
 *	\param[in]	data	Point to data.
 *	\param[in]	size	Data size in bytes.
 *	\return 			Zero on success or pointer to constant string with error description on error.
 */
char *dhap_post_parse(const char *data, unsigned int len);

#endif /* _DHAP_POST_H_ */
