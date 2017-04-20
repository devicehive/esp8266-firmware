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
#define FIRMWARE_VERSION "0.6"
/** Git revision */
#ifndef FIRMWARE_GIT_REVISION
	#define FIRMWARE_GIT_REVISION "unknown"
#endif
/** Buffer size for data that uses for commands which require data transmission via interfaces like UART, I2C etc. */
#define INTERFACES_BUF_SIZE 264
/** Encode type for data field in commands. Can be DATAENCODEBASE64 or DATAENCODEHEX.*/
#define DATAENCODEBASE64
/** SSID of Wi-Fi network for wireless configuration. */
#define WIFI_CONFIGURATION_SSID "DeviceHive"
/** Default playgroung url */
#define DEFAULT_SERVER "http://playground.devicehive.com/api/rest"
/** Interval of notification for. */
#define CUSTOM_NOTIFICATION_INTERVAL_MS 180000
/** Name for discovering in mDNS. Not more then 60 chars*/
#define MDNS_SERVICE_NAME "_esp8266-devicehive._tcp"
/** HTTP webserver and RESTful service port. */
#define HTTPD_PORT 80

// customize command set
#define DH_COMMANDS_GPIO // enable GPIO commands
#define DH_COMMANDS_ADC  // enable ADC commands
#define DH_COMMANDS_PWM  // enable PWM commands
#define DH_COMMANDS_UART // enable UART commands
#define DH_COMMANDS_I2C  // enable I2C commands
#define DH_COMMANDS_SPI  // enable SPI commands

#endif /* _USER_CONFIG_H_ */
