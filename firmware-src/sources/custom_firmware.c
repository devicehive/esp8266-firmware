/*
 * custom_firmware.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 */

#include "dhdebug.h"
#include "dhgpio.h"
#include "dhrequest.h"
#include "dhconnector.h"
#include "snprintf.h"
#include "c_types.h"
#include "osapi.h"

#include "devices/ds18b20.h"
#include "devices/bmp180.h"
#include "devices/dht.h"
#include "custom_firmware.h"

LOCAL HTTP_REQUEST notification;
int dht11_detected = 0;

void ICACHE_FLASH_ATTR test_output() {
	int dht11_temperature;
	float dht22_temperature;
	dhgpio_write(1 << CUSTOM_POWER_PIN, 0); // power up sensors
	os_delay_us(500000); //wait sensors
	float ds18b20_temperature = ds18b20_read(CUSTOM_DS18B20_PIN);
	int dht11_humiduty = dht11_read(CUSTOM_DHT11_PIN, &dht11_temperature);
	float dht22_humiduty = dht22_read(CUSTOM_DHT22_PIN, &dht22_temperature);
	float bmp180_temperature;
	int bmp180_pressure = bmp180_read(CUSTOM_BMP180_SDA_PIN,
			CUSTOM_BMP180_SCL_PIN, &bmp180_temperature);

	dhdebug("-----------------------------");
	dhdebug("ds18b20 temperature = %f C", ds18b20_temperature);
	dhdebug("dht11 humidity %d %%, temperature %d C", dht11_humiduty, dht11_temperature);
	dhdebug("dht22 humidity %f %%, temperature %f C", dht22_humiduty, dht22_temperature);
	dhdebug("bmp180 pressure %d Pa, temperature %f C", bmp180_pressure, bmp180_temperature);
	dhdebug("-----------------------------");
	dhgpio_write(0, 1 << CUSTOM_POWER_PIN); // power down sensors
	dhgpio_initialize(1 << CUSTOM_POWER_PIN, 0, 0);
}

HTTP_REQUEST * ICACHE_FLASH_ATTR custom_firmware_request() {
	dhdebug("Prepare sensors data...");
	dhgpio_write(1 << CUSTOM_POWER_PIN, 0); // power up sensors
	os_delay_us(500000); //wait dht11
	float temperature;
	float humiduty = dht22_read(CUSTOM_DHT22_PIN, &temperature);
	if (humiduty == DHT_ERROR) {
		temperature = ds18b20_read(CUSTOM_DS18B20_PIN);
		// recover if ds18b20 pin equal gpio1
		dhuart_init(UART_BAUND_RATE, 8, 'N', 1);
		int attemps = 5;
		while (humiduty == DHT_ERROR && --attemps) {
			humiduty = (float)dht11_read(CUSTOM_DHT11_PIN, 0);
			if(dht11_detected == 0)
				break;
			if(humiduty == DHT_ERROR)
				os_delay_us(100000);
			else
				dht11_detected = 1;
		}
	}
	int pressure = bmp180_read(CUSTOM_BMP180_SDA_PIN, CUSTOM_BMP180_SCL_PIN, 0);
	dhgpio_write(0, 1 << CUSTOM_POWER_PIN); // power down sensors
	dhgpio_initialize(1 << CUSTOM_POWER_PIN, 0, 0);
	char json[HTTP_REQUEST_MIN_ALLOWED_PAYLOAD] = "{";
	char *jsonp = json + 1;
	int comma = 0;
	if (temperature != DS18B20_ERROR) {
		jsonp += snprintf(jsonp, sizeof(json) - (jsonp - json) - 1, "\"temperature\":%f", temperature);
		comma = 1;
	}
	if (humiduty != DHT_ERROR) {
		jsonp += snprintf(jsonp, sizeof(json) - (jsonp - json) - 1, "%s\"humiduty\":%f", (comma ? ", ":""), humiduty);
		comma = 1;
	}
	if (pressure != BMP180_ERROR) {
		jsonp += snprintf(jsonp, sizeof(json) - (jsonp - json) - 1, "%s\"pressure\":%d", (comma ? ", ":""), pressure);
	}
	*jsonp ='}';
	*(jsonp + 1) = 0;
	dhrequest_create_notification(&notification, "climate", json);
	return &notification;
}
