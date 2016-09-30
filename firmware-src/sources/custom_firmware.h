/**
 *	\file		custom_firmware.h
 *	\brief		Custom firmware implementation.
 *	\details 	Module for creating custom firmwares which sends notifications time by time. Climate implementation.
 *	\author		Nikolay Khabarov
 *	\date		2016
 *	\copyright	DeviceHive MIT
 */
#ifndef SOURCES_CUSTOM_FIRMWARE_H_
#define SOURCES_CUSTOM_FIRMWARE_H_


/** Pin for custom hardware */
#define CUSTOM_DS18B20_PIN 1
#define CUSTOM_DHT11_PIN 2
#define CUSTOM_DHT22_PIN 5
#define CUSTOM_BMP180_SDA_PIN 12
#define CUSTOM_BMP180_SCL_PIN 14
#define CUSTOM_POWER_PIN 4

/**
 *	\brief				Print test output to debug.
 */
void test_output();


#endif /* SOURCES_CUSTOM_FIRMWARE_H_ */
