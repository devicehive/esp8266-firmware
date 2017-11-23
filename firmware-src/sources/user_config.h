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
#define FIRMWARE_VERSION "0.8"
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

// customize supported devices (compile time)
#ifndef DH_NO_IMPLICIT_DEVICES
#define DH_DEVICE_DS18B20
#define DH_DEVICE_DHT11
#define DH_DEVICE_DHT22
#define DH_DEVICE_BMP180
#define DH_DEVICE_BMP280
#define DH_DEVICE_BH1750
#define DH_DEVICE_MPU6050
#define DH_DEVICE_HMC5883L
#define DH_DEVICE_PCF8574
#define DH_DEVICE_PCF8574_HD44780
#define DH_DEVICE_MHZ19
#define DH_DEVICE_LM75
#define DH_DEVICE_SI7021
#define DH_DEVICE_ADS1115
#define DH_DEVICE_PCF8591
#define DH_DEVICE_MCP4725
#define DH_DEVICE_INA219
#define DH_DEVICE_MFRC522
#define DH_DEVICE_PCA9685
#define DH_DEVICE_MLX90614
#define DH_DEVICE_MAX6675
#define DH_DEVICE_MAX31855
#define DH_DEVICE_TM1637
#endif // DH_NO_IMPLICIT_DEVICES

// customize device commands (compile time)
#define DH_COMMANDS_DS18B20
#define DH_COMMANDS_DHT11
#define DH_COMMANDS_DHT22
#define DH_COMMANDS_BMP180
#define DH_COMMANDS_BMP280
#define DH_COMMANDS_BH1750
#define DH_COMMANDS_MPU6050
#define DH_COMMANDS_HMC5883L
#define DH_COMMANDS_PCF8574
#define DH_COMMANDS_PCF8574_HD44780
#define DH_COMMANDS_MHZ19
#define DH_COMMANDS_LM75
#define DH_COMMANDS_SI7021
#define DH_COMMANDS_ADS1115
#define DH_COMMANDS_PCF8591
#define DH_COMMANDS_MCP4725
#define DH_COMMANDS_INA219
#define DH_COMMANDS_MFRC522
#define DH_COMMANDS_PCA9685
#define DH_COMMANDS_MLX90614
#define DH_COMMANDS_MAX6675
#define DH_COMMANDS_MAX31855
#define DH_COMMANDS_TM1637

// customize command set (compile time)
#define DH_COMMANDS_GPIO     // enable GPIO commands
#define DH_COMMANDS_ADC      // enable ADC commands
#define DH_COMMANDS_PWM      // enable PWM commands
#define DH_COMMANDS_UART     // enable UART commands
#define DH_COMMANDS_I2C      // enable I2C commands
#define DH_COMMANDS_SPI      // enable SPI commands
#define DH_COMMANDS_ONEWIRE  // enable onewire commands

// allow to use secure connections to server
#define DH_USE_SSL

#endif /* _USER_CONFIG_H_ */
