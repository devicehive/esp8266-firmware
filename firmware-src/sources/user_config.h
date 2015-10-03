/**
 *	\file		user_config.h
 *	\brief		DeviceHive ESP8266 firmware main config file.
 *	\author		Nikolay Khabarov
 *	\date		2015
 *	\copyright	DeviceHive MIT
 */

#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

/** Interval in milliseconds that system should wait if error occupied. */
#define RETRY_CONNECTION_INTERVAL_MS 5000
/** Interval in milliseconds that system should after each request. This time needed for system to handle all wireless interruptions. */
#define DHREQUEST_PAUSE_MS 10
/** UART speed to terminal. */
#define UART_BAUND_RATE 115200
/** Current firmware version. */
#define FIRMWARE_VERSION "0.4"
/** Buffer size for data that uses for commands which require data transmition via interfaces like UART, I2C etc. */
#define INTERFACES_BUF_SIZE 264
/** Encode type for data field in commands. Can be DATAENCODEBASE64 or DATAENCODEHEX.*/
#define DATAENCODEBASE64
/** SSID of Wi-Fi network for wireless configuration. */
#define WIFI_CONFIGURATION_SSID "DeviceHive"
/** Default playgroung url */
#define DEFAULT_SERVER "http://playground.devicehive.com/api/rest"

#endif /* _USER_CONFIG_H_ */
