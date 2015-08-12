/**
 *	\file		dhesperrors.h
 *	\brief		Print human readable error in debug.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef USER_DHESPERRORS_H_
#define USER_DHESPERRORS_H_

/**
 *	\brief					Print disconnect reason.
 *	\param[in]	descrption	Text that will be printed before error description.
 *	\param[in]	reason		Disconnect reason.
 */
void dhesperrors_disconnect_reason(const char *descrption, uint8 reason);

/**
 *	\brief					Print espconn result.
 *	\param[in]	descrption	Text that will be printed before error description.
 *	\param[in]	reason		espconn result.
 */
void dhesperrors_espconn_result(const char *descrption, int reason);

#endif /* USER_DHESPERRORS_H_ */
