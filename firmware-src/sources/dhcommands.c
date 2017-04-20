/*
 * dhcommands.cpp
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for executing server command
 *
 */
#include "dhcommands.h"
#include "dhsender_queue.h"
#include "DH/gpio.h"
#include "DH/adc.h"
#include "DH/uart.h"
#include "dhnotification.h"
#include "snprintf.h"
#include "dhcommand_parser.h"
#include "dhterminal.h"
#include "dhi2c.h"
#include "dhspi.h"
#include "dhonewire.h"
#include "dhdebug.h"
#include "DH/pwm.h"
#include "dhutils.h"
#include "devices/ds18b20.h"
#include "devices/dht.h"
#include "devices/bmp180.h"
#include "devices/bmp280.h"
#include "devices/bh1750.h"
#include "devices/mpu6050.h"
#include "devices/hmc5883l.h"
#include "devices/pcf8574.h"
#include "devices/pcf8574_hd44780.h"
#include "devices/mhz19.h"
#include "devices/lm75.h"
#include "devices/si7021.h"
#include "devices/ads1115.h"
#include "devices/pcf8591.h"
#include "devices/mcp4725.h"
#include "devices/ina219.h"
#include "devices/mfrc522.h"
#include "devices/pca9685.h"
#include "devices/mlx90614.h"
#include "devices/max6675.h"
#include "devices/max31855.h"
#include "devices/tm1636.h"

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <user_interface.h>
#include <ets_forward.h>

LOCAL int ICACHE_FLASH_ATTR onewire_init(COMMAND_RESULT *cb, ALLOWED_FIELDS fields, gpio_command_params *parse_pins) {
	if(fields & AF_PIN) {
		if(dhonewire_set_pin(parse_pins->pin) == 0) {
			dh_command_fail(cb, "Wrong onewire pin");
			return 1;
		}
	}
	return 0;
}

LOCAL int ICACHE_FLASH_ATTR spi_init(COMMAND_RESULT *cb, ALLOWED_FIELDS fields, gpio_command_params *parse_pins) {
	if(fields & AF_CS) {
		if(dhspi_set_cs_pin(parse_pins->CS) == 0) {
			dh_command_fail(cb, "Wrong CS pin");
			return 1;
		}
	}
	if(fields & AF_SPIMODE) {
		if(dhspi_set_mode(parse_pins->spi_mode) == 0) {
			dh_command_fail(cb, "Wrong SPI mode");
			return 1;
		}
	}
	return 0;
}

LOCAL char *ICACHE_FLASH_ATTR i2c_status_tochar(DHI2C_STATUS status) {
	switch(status) {
		case DHI2C_NOACK:
			return "ACK response not detected";
		case DHI2C_WRONG_PARAMETERS:
			return "Wrong parameters";
		case DHI2C_BUS_BUSY:
			return "Bus is busy";
		case DHI2C_DEVICE_ERROR:
			return "Device error";
		case DHI2C_OK:
			break;
	}
	return 0;
}

LOCAL int ICACHE_FLASH_ATTR i2c_init(COMMAND_RESULT *cb, ALLOWED_FIELDS fields, gpio_command_params *parse_pins) {
	if((fields & AF_ADDRESS) == 0) {
		dh_command_fail(cb, "Address not specified");
		return 1;
	}
	int init = ((fields & AF_SDA) ? 1 : 0) + ((fields & AF_SCL) ? 1 : 0);
	if(init == 2) {
		char *res = i2c_status_tochar(dhi2c_init(parse_pins->SDA, parse_pins->SCL));
		if (res != 0) {
			dh_command_fail(cb, res);
			return 1;
		}
	} else if(init == 1) {
		dh_command_fail(cb, "Only one pin specified");
		return 1;
	} else {
		dhi2c_reinit();
	}
	return 0;
}

#if 1 // I2C commands

/**
 * @brief Do "i2c/master/read" command.
 */
static void ICACHE_FLASH_ATTR do_i2c_master_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_DATA | AF_ADDRESS | AF_COUNT, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if((fields & AF_COUNT) == 0)
		parse_pins.count = 2;
	if(parse_pins.count == 0 || parse_pins.count > INTERFACES_BUF_SIZE) {
		dh_command_fail(cb, "Wrong read size");
		return;
	}
	if(i2c_init(cb, fields, &parse_pins))
		return;
	char *res;
	if(fields & AF_DATA) {
		res = i2c_status_tochar(dhi2c_write(parse_pins.address, parse_pins.data, parse_pins.data_len, 0));
		if(res) {
			dh_command_fail(cb, res);
			return;
		}
	}
	res = i2c_status_tochar(dhi2c_read(parse_pins.address, parse_pins.data, parse_pins.count));
	if(res) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_DATA_WITH_LEN, parse_pins.data, parse_pins.count);
	}
}


/**
 * @brief Do "i2c/master/write" command.
 */
static void ICACHE_FLASH_ATTR do_i2c_master_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_DATA | AF_ADDRESS, &fields);
	if(parse_res) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if((fields & AF_DATA) == 0) {
		dh_command_fail(cb, "Data not specified");
		return;
	}
	if(i2c_init(cb, fields, &parse_pins))
		return;
	char *res = i2c_status_tochar(dhi2c_write(parse_pins.address, parse_pins.data, parse_pins.data_len, 1));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		dh_command_done(cb, "");
	}
}

#endif // I2C commands

#if 1 // SPI commands

/**
 * @brief Do "spi/master/read" command.
 */
static void ICACHE_FLASH_ATTR do_spi_master_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_CS | AF_SPIMODE | AF_DATA | AF_COUNT, &fields);
	if(parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if((fields & AF_COUNT) == 0)
		parse_pins.count = 2;
	if(parse_pins.count == 0 || parse_pins.count > INTERFACES_BUF_SIZE) {
		dh_command_fail(cb, "Wrong read size");
		return;
	}
	if(spi_init(cb, fields, &parse_pins))
		return;
	if(fields & AF_DATA)
		dhspi_write(parse_pins.data, parse_pins.data_len, 0);
	dhspi_read(parse_pins.data, parse_pins.count);
	cb->callback(cb->data, DHSTATUS_OK, RDT_DATA_WITH_LEN, parse_pins.data, parse_pins.count);
}


/**
 * @brief Do "spi/master/write" command.
 */
static void ICACHE_FLASH_ATTR do_spi_master_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_CS | AF_SPIMODE | AF_DATA, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if(spi_init(cb, fields, &parse_pins))
		return;
	if((fields & AF_DATA) == 0) {
		dh_command_fail(cb, "Data not specified");
		return;
	}
	dhspi_write(parse_pins.data, parse_pins.data_len, 1);
	dh_command_done(cb, "");
}

#endif // SPI commands

#if 1 // onewire commands

/**
 * @brief Do "onewire/master/read" command.
 */
static void ICACHE_FLASH_ATTR do_onewire_master_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_PIN | AF_DATA | AF_COUNT, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if((fields & AF_COUNT) == 0 || parse_pins.count == 0 || parse_pins.count > INTERFACES_BUF_SIZE) {
		dh_command_fail(cb, "Wrong read size");
		return;
	}
	if((fields & AF_DATA) == 0) {
		dh_command_fail(cb, "Command for reading is not specified");
		return;
	}
	if(onewire_init(cb, fields, &parse_pins))
		return;
	if(dhonewire_write(parse_pins.data, parse_pins.data_len) == 0) {
		dh_command_fail(cb, "No response");
		return;
	}
	dhonewire_read(parse_pins.data, parse_pins.count);
	cb->callback(cb->data, DHSTATUS_OK, RDT_DATA_WITH_LEN, parse_pins.data, parse_pins.count);
}


/**
 * @brief Do "onewire/master/write" commands.
 */
static void ICACHE_FLASH_ATTR do_onewire_master_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char * parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_PIN | AF_DATA, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if(onewire_init(cb, fields, &parse_pins))
		return;
	if(dhonewire_write(parse_pins.data, parse_pins.data_len) == 0) {
		dh_command_fail(cb, "No response");
		return;
	}
	dh_command_done(cb, "");
}

/**
 * Do "onewire/master/search" or "onewire/master/alarm" commands.
 */
static void ICACHE_FLASH_ATTR do_onewire_master_search(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;

	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_PIN, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(onewire_init(cb, fields, &parse_pins))
			return;
	}
	parse_pins.data_len = sizeof(parse_pins.data) ;

	int check = os_strcmp(command, "onewire/master/search");
	if(dhonewire_search(parse_pins.data, (unsigned long *)&parse_pins.data_len, (check == 0) ? 0xF0 : 0xEC, dhonewire_get_pin()))
		cb->callback(cb->data, DHSTATUS_OK, RDT_SEARCH64, dhonewire_get_pin(), parse_pins.data, parse_pins.data_len);
	else
		dh_command_fail(cb, "Error during search");
}


/**
 * @brief Do "onewire/master/int" command.
 */
static void ICACHE_FLASH_ATTR do_onewire_master_int(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;

	char *parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DH_GPIO_SUITABLE_PINS, dh_gpio_get_timeout(), AF_DISABLE | AF_PRESENCE, &fields);
	if(parse_res)
		dh_command_fail(cb, parse_res);
	else if(fields == 0)
		dh_command_fail(cb, "Wrong action");
	else if(dhonewire_int(parse_pins.pins_to_presence, parse_pins.pins_to_disable))
		dh_command_done(cb, "");
	else
		dh_command_fail(cb, "Unsuitable pin");
}


/**
 * @brief Do "onewire/dht/read" command.
 */
static void ICACHE_FLASH_ATTR do_onewire_dht_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_PIN, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(onewire_init(cb, fields, &parse_pins))
			return;
	}
	parse_pins.count = dhonewire_dht_read(parse_pins.data, sizeof(parse_pins.data));
	if(parse_pins.count)
		cb->callback(cb->data, DHSTATUS_OK, RDT_DATA_WITH_LEN, parse_pins.data, parse_pins.count);
	else
		dh_command_fail(cb, "No response");
}


/**
 * @brief Do "onewire/ws2812b/write" command.
 */
static void ICACHE_FLASH_ATTR do_onewire_ws2812b_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;

	char *parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_PIN | AF_DATA, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if((fields & AF_DATA) == 0) {
		dh_command_fail(cb, "Data not specified");
		return;
	}
	if(onewire_init(cb, fields, &parse_pins))
		return;
	dhonewire_ws2812b_write(parse_pins.data, parse_pins.data_len);
	dh_command_done(cb, "");
}

#endif // onewire commands

#if 1 // devices commands

/**
 * @brief Do "devices/ds18b20/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_ds18b20_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	if(paramslen) {
		gpio_command_params parse_pins;
		ALLOWED_FIELDS fields = 0;
		char * parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_PIN, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(onewire_init(cb, fields, &parse_pins))
			return;
	}
	float temperature;
	char *res = ds18b20_read(DS18B20_NO_PIN, &temperature);
	if(res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%f}", temperature);
	}
}


/**
 * @brief Do "devices/dht11/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_dht11_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	if(paramslen) {
		gpio_command_params parse_pins;
		ALLOWED_FIELDS fields = 0;
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_PIN, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(onewire_init(cb, fields, &parse_pins))
			return;
	}

	int temperature;
	int humidity;
	char *res = dht11_read(DHT_NO_PIN, &humidity, &temperature);
	if(res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%d, \"humidity\":%d}", temperature, humidity);
	}
}


/**
 * @brief Do "devices/dht22/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_dht22_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	if(paramslen) {
		gpio_command_params parse_pins;
		ALLOWED_FIELDS fields = 0;
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_PIN, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(onewire_init(cb, fields, &parse_pins))
			return;
	}

	float temperature;
	float humidity;
	char *res = dht22_read(DHT_NO_PIN, &humidity, &temperature);
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%f, \"humidity\":%f}", temperature, humidity);
	}
}

/**
 * @brief Do "devices/bmp180/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_bmp180_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			bmp180_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	float temperature;
	int pressure;
	char *res = i2c_status_tochar(bmp180_read(BMP180_NO_PIN, BMP180_NO_PIN, &pressure, &temperature));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%f, \"pressure\":%d}", temperature, pressure);
	}
}

/**
 * @brief Do "devices/bmp280/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_bmp280_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			bmp280_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	float temperature;
	float pressure;
	char *res = i2c_status_tochar(bmp280_read(BMP280_NO_PIN, BMP280_NO_PIN, &pressure, &temperature));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%f, \"pressure\":%f}", temperature, pressure);
	}
}


/**
 * @brief Do "devices/bh1750/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_bh1750_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			bh1750_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	float illuminance;
	char *res = i2c_status_tochar(bh1750_read(BH1750_NO_PIN, BH1750_NO_PIN, &illuminance));
	if(res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"illuminance\":%f}", illuminance);
	}
}

/**
 * @brief Do "devices/mpu6050/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_mpu6050_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			mpu6050_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	MPU6050_XYZ acc;
	MPU6050_XYZ gyro;
	float temperature;
	char *res = i2c_status_tochar(mpu6050_read(MPU6050_NO_PIN, MPU6050_NO_PIN, &acc, &gyro, &temperature));
	if(res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING,
				"{\"temperature\":%f, \"acceleration\":{\"X\":%f, \"Y\":%f, \"Z\":%f}, \"rotation\":{\"X\":%f, \"Y\":%f, \"Z\":%f}}",
				temperature, acc.X, acc.Y, acc.Z, gyro.X, gyro.Y, gyro.Z);
	}
}

/**
 * @brief Do "devices/hmc5883l/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_hmc5883l_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			hmc5883l_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	HMC5883L_XYZ compass;
	char *res = i2c_status_tochar(hmc5883l_read(HMC5883L_NO_PIN, HMC5883L_NO_PIN, &compass));
	if(res != 0) {
		dh_command_fail(cb, res);
		return;
	}
	char floatbufx[10] = "NaN";
	char floatbufy[10] = "NaN";
	char floatbufz[10] = "NaN";
	if(compass.X != HMC5883l_OVERFLOWED)
		snprintf(floatbufx, sizeof(floatbufx), "%f", compass.X);
	if(compass.Y != HMC5883l_OVERFLOWED)
		snprintf(floatbufy, sizeof(floatbufy), "%f", compass.Y);
	if(compass.Z != HMC5883l_OVERFLOWED)
		snprintf(floatbufz, sizeof(floatbufz), "%f", compass.Z);
	cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING,
	    "{\"magnetometer\":{\"X\":%s, \"Y\":%s, \"Z\":%s}}",
	    floatbufx, floatbufy, floatbufz);
}

/**
 * @brief Do "devices/pcf8574/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_pcf8574_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, PCF8574_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS | AF_PULLUP, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			pcf8574_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	if(fields & AF_PULLUP) {
		char *res = i2c_status_tochar(pcf8574_write(PCF8574_NO_PIN, PCF8574_NO_PIN, parse_pins.pins_to_pullup, 0));
		if (res != 0) {
			dh_command_fail(cb, res);
			return;
		}
	}
	unsigned int pins;
	char *res = i2c_status_tochar(pcf8574_read(PCF8574_NO_PIN, PCF8574_NO_PIN, &pins));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_GPIO, 0, pins, system_get_time(), PCF8574_SUITABLE_PINS);
	}
}

/**
 * @brief Do "devices/pcf8574/write" command.
 */
static void ICACHE_FLASH_ATTR do_devices_pcf8574_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, PCF8574_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_SET | AF_CLEAR, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	} else if( (fields & (AF_SET | AF_CLEAR)) == 0) {
		dh_command_fail(cb, "Dummy request");
		return;
	} else if( (parse_pins.pins_to_set | parse_pins.pins_to_clear | PCF8574_SUITABLE_PINS)
	        != PCF8574_SUITABLE_PINS ) {
		dh_command_fail(cb, "Unsuitable pin");
		return;
	}
	if(fields & AF_ADDRESS)
		pcf8574_set_address(parse_pins.address);
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	char *res = i2c_status_tochar(pcf8574_write(PCF8574_NO_PIN, PCF8574_NO_PIN,
	        parse_pins.pins_to_set, parse_pins.pins_to_clear));
	if(res != 0) {
		dh_command_fail(cb, res);
	} else {
		dh_command_done(cb, "");
	}
}

/**
 * @brief Do "devices/pcf8574/hd44780/write" command.
 */
static void ICACHE_FLASH_ATTR do_devices_pcf8574_hd44780_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, PCF8574_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_DATA | AF_TEXT_DATA, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if((fields & (AF_DATA | AF_TEXT_DATA)) == 0 || parse_pins.data_len == 0) {
		dh_command_fail(cb, "Text not specified");
		return;
	}
	if(fields & AF_ADDRESS)
		pcf8574_set_address(parse_pins.address);
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	char *res = i2c_status_tochar(pcf8574_hd44780_write(PCF8574_NO_PIN, PCF8574_NO_PIN,
	        parse_pins.data, parse_pins.data_len));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		dh_command_done(cb, "");
	}
}

/**
 * @brief Do "devices/mhz19/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_mhz19_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	if(paramslen) {
		dh_command_fail(cb, "Command does not have parameters");
		return;
	}
	int co2;
	char *res = mhz19_read(&co2);
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"co2\":%d}", co2);
	}
}


/**
 * @brief Do "devices/lm75/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_lm75_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			lm75_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	float temperature;
	char *res = i2c_status_tochar(lm75_read(LM75_NO_PIN, LM75_NO_PIN, &temperature));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%f}", temperature);
	}
}


/**
 * @brief Do "devices/si7021/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_si7021_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			si7021_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	float temperature;
	float humidity;
	char *res = i2c_status_tochar(si7021_read(SI7021_NO_PIN, SI7021_NO_PIN, &humidity, &temperature));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%f, \"humidity\":%f}", temperature, humidity);
	}
}

/**
 * @brief Do "devices/ads1115/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_ads1115_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			ads1115_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	float values[4];
	char *res = i2c_status_tochar(ads1115_read(ADS1115_NO_PIN, ADS1115_NO_PIN, values));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"0\":%f, \"1\":%f, \"2\":%f, \"3\":%f}",
				values[0], values[1], values[2], values[3]);
	}
}

/**
 * @brief Do "devices/pcf8591/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_pcf8591_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS | AF_REF, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			pcf8591_set_address(parse_pins.address);
		if(fields & AF_REF) {
			char *res = i2c_status_tochar(pcf8591_set_vref(parse_pins.ref));
			if (res != 0) {
				dh_command_fail(cb, res);
				return;
			}
		}
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	float values[4];
	char *res = i2c_status_tochar(pcf8591_read(ADS1115_NO_PIN, ADS1115_NO_PIN, values));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"0\":%f, \"1\":%f, \"2\":%f, \"3\":%f}",
				values[0], values[1], values[2], values[3]);
	}
}


/**
 * @brief Do "devices/pcf8591/write" command.
 */
static void ICACHE_FLASH_ATTR do_devices_pcf8591_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS | AF_REF | AF_FLOATVALUES, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if(parse_pins.pin_value_readed != 1) {
		dh_command_fail(cb, "Unsuitable pin");
		return;
	}
	if(fields & AF_ADDRESS)
		pcf8591_set_address(parse_pins.address);
	if(fields & AF_REF) {
		char *res = i2c_status_tochar(pcf8591_set_vref(parse_pins.ref));
		if (res != 0) {
			dh_command_fail(cb, res);
			return;
		}
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	char *res = i2c_status_tochar(pcf8591_write(MCP4725_NO_PIN, MCP4725_NO_PIN, parse_pins.storage.float_values[0]));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		dh_command_done(cb, "");
	}
}

/**
 * @brief Do "devices/mcp4725/write" command.
 */
static void ICACHE_FLASH_ATTR do_devices_mcp4725_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_REF | AF_FLOATVALUES, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if(parse_pins.pin_value_readed != 1) {
		dh_command_fail(cb, "Unsuitable pin");
		return;
	}
	if(fields & AF_ADDRESS)
		mcp4725_set_address(parse_pins.address);
	if(fields & AF_REF) {
		char *res = i2c_status_tochar(mcp4725_set_vref(parse_pins.ref));
		if (res != 0) {
			dh_command_fail(cb, res);
			return;
		}
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	char *res = i2c_status_tochar(mcp4725_write(MCP4725_NO_PIN, MCP4725_NO_PIN, parse_pins.storage.float_values[0]));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		dh_command_done(cb, "");
	}
}

/**
 * @brief Do "devices/ina219/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_ina219_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
					&parse_pins, DH_ADC_SUITABLE_PINS, 0,
					AF_SDA | AF_SCL | AF_ADDRESS | AF_REF, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			ina219_set_address(parse_pins.address);
		if(fields & AF_REF) {
			char *res = i2c_status_tochar(ina219_set_shunt(parse_pins.ref));
			if (res != 0) {
				dh_command_fail(cb, res);
				return;
			}
		}
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	float voltage;
	float current;
	float power;
	char *res = i2c_status_tochar(ina219_read(ADS1115_NO_PIN, ADS1115_NO_PIN, &voltage, &current, &power));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING,
				"{\"voltage\":%f, \"current\":%f, \"power\":%f}",
				voltage, current, power);
	}
}

/**
 * @brief Do "devices/mfrc522/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_mfrc522_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	if(paramslen) {
		gpio_command_params parse_pins;
		ALLOWED_FIELDS fields = 0;
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_CS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_CS) {
			if(MFRC522_Set_CS(parse_pins.CS) != MFRC522_STATUS_OK) {
				dh_command_fail(cb, "Unsuitable pin");
				return;
			}
		}
	}
	MFRC522_PCD_Init();
	uint8_t bufferATQA[2];
	uint8_t bufferSize = sizeof(bufferATQA);
	MFRC522_StatusCode result = MFRC522_PICC_RequestA(bufferATQA, &bufferSize);
	if(result == MFRC522_STATUS_OK || result == MFRC522_STATUS_COLLISION) {
		MFRC522_Uid *uid = MFRC522_Get_Uid();
		result = MFRC522_PICC_Select(uid, 0);
		if(result == MFRC522_STATUS_OK) {
			char hexbuf[uid->size * 2 + 1];
			unsigned int i;
			for(i = 0; i < uid->size; i++)
			byteToHex(uid->uidByte[i], &hexbuf[i * 2]);
			hexbuf[sizeof(hexbuf) - 1] = 0;
			cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING,
			                "{\"uid\":\"0x%s\", \"type\":\"%s\"}", hexbuf,
			                MFRC522_PICC_GetTypeName(MFRC522_PICC_GetType(uid->sak)));
			MFRC522_PCD_AntennaOff();
			return;
		}
	}
	MFRC522_PICC_HaltA();
	MFRC522_PCD_AntennaOff();
	dh_command_fail(cb, MFRC522_GetStatusCodeName(result));
}

/**
 * @brief Do "devices/mfrc522/mifare/read" and "devices/mfrc522/mifare/write" commands.
 */
static void ICACHE_FLASH_ATTR do_devices_mfrc522_mifare_read_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	const int check = os_strcmp(command, "devices/mfrc522/mifare/read");
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_CS | AF_ADDRESS | AF_KEY | (check ? AF_DATA : 0), &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if(fields & AF_CS) {
		if(MFRC522_Set_CS(parse_pins.CS) != MFRC522_STATUS_OK) {
			dh_command_fail(cb, "Unsuitable pin");
			return;
		}
	}
	if((fields & AF_ADDRESS) == 0) {
		dh_command_fail(cb, "Block address not specified");
		return;
	}
	if((fields & AF_KEY) == 0) {
		// default key
		os_memset(parse_pins.storage.key.key_data, 0xFF, MF_KEY_SIZE);
		parse_pins.storage.key.key_len = MF_KEY_SIZE;
	} else if(parse_pins.storage.key.key_len != MF_KEY_SIZE) {
		dh_command_fail(cb, "Wrong key length");
		return;
	}
	if(check) {
		if((fields & AF_DATA) == 0) {
			dh_command_fail(cb, "Data not specified");
			return;
		} else if(parse_pins.data_len != 16) {
			dh_command_fail(cb, "Data length should be 16 bytes");
			return;
		}
	}
	MFRC522_PCD_Init();
	uint8_t bufferATQA[2];
	uint8_t bufferSize = sizeof(bufferATQA);
	MFRC522_StatusCode result = MFRC522_PICC_RequestA(bufferATQA, &bufferSize);
	if(result == MFRC522_STATUS_OK || result == MFRC522_STATUS_COLLISION) {
		MFRC522_Uid *uid = MFRC522_Get_Uid();
		result = MFRC522_PICC_Select(uid, 0);
		MIFARE_Key key;
		os_memcpy(key.keyByte, parse_pins.storage.key.key_data, MF_KEY_SIZE);
		if(result == MFRC522_STATUS_OK) {
			result = MFRC522_PCD_Authenticate(PICC_CMD_MF_AUTH_KEY_A, parse_pins.address, &key, uid);
			if(result == MFRC522_STATUS_OK) {
				uint8_t len = (sizeof(parse_pins.data) > 0xFF) ? 0xFF : sizeof(parse_pins.data);
				if(check)
					result = MFRC522_MIFARE_Write(parse_pins.address, parse_pins.data, parse_pins.data_len);
				else
					result = MFRC522_MIFARE_Read(parse_pins.address, parse_pins.data, &len);
				if(result == MFRC522_STATUS_OK) {
					parse_pins.count = len;
					if(check)
						dh_command_done(cb, "");
					else
						cb->callback(cb->data, DHSTATUS_OK, RDT_DATA_WITH_LEN, parse_pins.data, parse_pins.count);
					MFRC522_PICC_HaltA();
					MFRC522_PCD_StopCrypto1();
					MFRC522_PCD_AntennaOff();
					return;
				}
			}
		}
	}
	MFRC522_PICC_HaltA();
	MFRC522_PCD_StopCrypto1();
	MFRC522_PCD_AntennaOff();
	dh_command_fail(cb, MFRC522_GetStatusCodeName(result));
}

/**
 * @brief Do "devices/pca9685/control" command.
 */
static void ICACHE_FLASH_ATTR do_devices_pca9685_control(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, PCA9685_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_ADDRESS | AF_FLOATVALUES | AF_PERIOD, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if(fields & AF_ADDRESS)
		pca9685_set_address(parse_pins.address);
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	char *res = i2c_status_tochar(pca9685_control(PCA9685_NO_PIN, PCA9685_NO_PIN,
	        parse_pins.storage.float_values, parse_pins.pin_value_readed,
	        (fields & AF_PERIOD) ? parse_pins.periodus : PCA9685_NO_PERIOD));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		dh_command_done(cb, "");
	}
}

/**
 * @brief Do "devices/mlx90614/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_mlx90614_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0,
				AF_SDA | AF_SCL | AF_ADDRESS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
		if(fields & AF_ADDRESS)
			mlx90614_set_address(parse_pins.address);
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	float ambient;
	float object;
	char *res = i2c_status_tochar(mlx90614_read(MLX90614_NO_PIN, MLX90614_NO_PIN, &ambient, &object));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"ambient\":%f, \"object\":%f}", ambient, object);
	}
}

/**
 * @brief Do "devices/max6675/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_max6675_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_CS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
	}
	float temperature;
	char *res = max6675_read((fields & AF_CS) ? parse_pins.CS : MAX6675_NO_PIN, &temperature);
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%f}", temperature);
	}
}

/**
 * @brief Do "devices/max31855/read" command.
 */
static void ICACHE_FLASH_ATTR do_devices_max31855_read(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	if(paramslen) {
		char *parse_res = parse_params_pins_set(params, paramslen,
				&parse_pins, DH_ADC_SUITABLE_PINS, 0, AF_CS, &fields);
		if (parse_res != 0) {
			dh_command_fail(cb, parse_res);
			return;
		}
	}
	float temperature;
	char *res = max31855_read((fields & AF_CS) ? parse_pins.CS : MAX31855_NO_PIN, &temperature);
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		cb->callback(cb->data, DHSTATUS_OK, RDT_FORMAT_STRING, "{\"temperature\":%f}", temperature);
	}
}

/**
 * @brief Do "devices/tm1637/write" command.
 */
static void ICACHE_FLASH_ATTR do_devices_tm1637_write(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen)
{
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res = parse_params_pins_set(params, paramslen,
			&parse_pins, DH_ADC_SUITABLE_PINS, 0,
			AF_SDA | AF_SCL | AF_DATA | AF_TEXT_DATA, &fields);
	if (parse_res != 0) {
		dh_command_fail(cb, parse_res);
		return;
	}
	if((fields & (AF_DATA | AF_TEXT_DATA)) == 0 || parse_pins.data_len == 0) {
		dh_command_fail(cb, "Text not specified");
		return;
	}
	fields |= AF_ADDRESS;
	if(i2c_init(cb, fields, &parse_pins))
		return;
	char *res = i2c_status_tochar(tm1636_write(TM1636_NO_PIN, TM1636_NO_PIN,
	        parse_pins.data, parse_pins.data_len));
	if (res != 0) {
		dh_command_fail(cb, res);
	} else {
		dh_command_done(cb, "");
	}
}

#endif // devices commands

RO_DATA struct {
	const char *name;
	void (*func)(COMMAND_RESULT*, const char*, const char*, unsigned int);
} g_command_table[] =
{
#ifdef DH_COMMANDS_GPIO
	{"gpio/write", dh_handle_gpio_write},
	{"gpio/read", dh_handle_gpio_read},
	{"gpio/int", dh_handle_gpio_int},
#endif /* DH_COMMANDS_GPIO */

#ifdef DH_COMMANDS_ADC
	{"adc/read", dh_handle_adc_read},
	{"adc/int", dh_handle_adc_int},
#endif /* DH_COMMANDS_ADC */

#ifdef DH_COMMANDS_PWM
	{"pwm/control", dh_handle_pwm_control},
#endif /* DH_COMMANDS_PWM */

#ifdef DH_COMMANDS_UART
	{"uart/write", dh_handle_uart_write},
	{"uart/read", dh_handle_uart_read},
	{"uart/int", dh_handle_uart_int},
	{"uart/terminal", dh_handle_uart_terminal},
#endif /* DH_COMMANDS_UART */

	{ "i2c/master/read", do_i2c_master_read},
	{ "i2c/master/write", do_i2c_master_write},

	{ "spi/master/read", do_spi_master_read},
	{ "spi/master/write", do_spi_master_write},

	{ "onewire/master/read", do_onewire_master_read},
	{ "onewire/master/write", do_onewire_master_write},
	{ "onewire/master/search", do_onewire_master_search},
	{ "onewire/master/alarm", do_onewire_master_search},
	{ "onewire/master/int", do_onewire_master_int},
	{ "onewire/dht/read", do_onewire_dht_read},
	{ "onewire/ws2812b/write", do_onewire_ws2812b_write},

	{ "devices/ds18b20/read", do_devices_ds18b20_read},
	{ "devices/dht11/read", do_devices_dht11_read},
	{ "devices/dht22/read", do_devices_dht22_read},
	{ "devices/bmp180/read", do_devices_bmp180_read},
	{ "devices/bmp280/read", do_devices_bmp280_read},
	{ "devices/bh1750/read", do_devices_bh1750_read},
	{ "devices/mpu6050/read", do_devices_mpu6050_read},
	{ "devices/hmc5883l/read", do_devices_hmc5883l_read},
	{ "devices/pcf8574/read", do_devices_pcf8574_read},
	{ "devices/pcf8574/write", do_devices_pcf8574_write},
	{ "devices/pcf8574/hd44780/write", do_devices_pcf8574_hd44780_write},
	{ "devices/mhz19/read", do_devices_mhz19_read},
	{ "devices/lm75/read", do_devices_lm75_read},
	{ "devices/si7021/read", do_devices_si7021_read},
	{ "devices/ads1115/read", do_devices_ads1115_read},
	{ "devices/pcf8591/read", do_devices_pcf8591_read},
	{ "devices/pcf8591/write", do_devices_pcf8591_write},
	{ "devices/mcp4725/write", do_devices_mcp4725_write},
	{ "devices/ina219/read", do_devices_ina219_read},
	{ "devices/mfrc522/read", do_devices_mfrc522_read},
	{ "devices/mfrc522/mifare/read", do_devices_mfrc522_mifare_read_write},
	{ "devices/mfrc522/mifare/write", do_devices_mfrc522_mifare_read_write},
	{ "devices/pca9685/control", do_devices_pca9685_control},
	{ "devices/mlx90614/read", do_devices_mlx90614_read},
	{ "devices/max6675/read", do_devices_max6675_read},
	{ "devices/max31855/read", do_devices_max31855_read},
	{ "devices/tm1637/write", do_devices_tm1637_write}
};


/**
 * @brief Number of commands in the table.
 */
#define NUM_OF_COMMANDS (sizeof(g_command_table)/sizeof(g_command_table[0]))


void ICACHE_FLASH_ATTR dhcommands_do(COMMAND_RESULT *cb, const char *command, const char *params, unsigned int paramslen) {
	int i;

	dhdebug("Got command: %s %d", command, cb->data.id);
	for (i = 0; i < NUM_OF_COMMANDS; ++i) {
		if (0 == os_strcmp(command, g_command_table[i].name)) {
			g_command_table[i].func(cb, command, params, paramslen);
			return; // done
		}
	}

	dh_command_fail(cb, "Unknown command");
}
