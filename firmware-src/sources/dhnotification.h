/**
 *	\file		dhnotification.h
 *	\brief		Notification help manager.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 *	\details	Collect callbacks from HAL modules and sends notifications.
 */

#ifndef USER_DHNOTIFICATION_H_
#define USER_DHNOTIFICATION_H_

/**
 *	\brief				Util function for converting GPIO pins status to DeviceHive JSON
 *	\param[out]	out		Buffer for output JSON, should be at least 192 bytes.
 *	\param		value	Bitwise GPIO pins state.
 *	\return				Number of character that was written to buffer.
 */
int dhnotification_gpio_read_to_json(char *out, unsigned int value);

/**
 *	\brief				Util function for preparing raw data into request JSON parameter.
 *	\param[out]	buf		Buffer for output JSON, should be at least 16 bytes.
 *	\param[in]	bufsize	Buffer size in bytes.
 *	\param[in]	data	Data for converting.
 *	\param[in]	len		Data size in bytes.
 *	\return				Zero on success or pointer to string with error description.
 */
char * dhnotification_prepare_data_parameters(char *buf, unsigned int bufsize, const char *data, unsigned int len);

#endif /* USER_DHNOTIFICATION_H_ */
