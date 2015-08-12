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

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include "dhcommands.h"
#include "dhsender.h"
#include "dhgpio.h"
#include "dhadc.h"
#include "dhnotification.h"
#include "snprintf.h"
#include "dhcommand_parser.h"
#include "dhterminal.h"
#include "dhuart.h"
#include "dhi2c.h"

#define GPIONOTIFICATION_MIN_TIMEOUT_MS 50
#define ADCNOTIFICATION_MIN_TIMEOUT_MS 250

LOCAL char *ICACHE_FLASH_ATTR i2c_status_tochar(DHI2C_STATUS status) {
	switch(status) {
		case DHI2C_NOACK:
			return "ACK response not detected";
		case DHI2C_WRONG_PARAMETERS:
			return "Wrong I2C parameters";
		case DHI2C_TIMEOUT:
			return "Can not set bus";
	}
	return 0;
}

LOCAL char *ICACHE_FLASH_ATTR i2c_init(ALLOWED_FIELDS fields, gpio_command_params *parse_pins) {
	if((fields & AF_ADDRESS) == 0)
		return "Address not specified";
	int init = ((fields & AF_SDA) ? 1 : 0) + ((fields & AF_SCL) ? 1 : 0);
	if(init == 2) {
		char *res = i2c_status_tochar(dhi2c_init(parse_pins->SDA, parse_pins->SCL));
		if(res)
			return res;
	} else if(init == 1) {
		return "Only one pin specified";
	} else {
		dhi2c_reinit();
	}
	return 0;
}

LOCAL int ICACHE_FLASH_ATTR uart_init(int id, ALLOWED_FIELDS fields, gpio_command_params *parse_pins) {
	if(fields & AF_UARTMODE) {
		if(parse_pins->uart_speed == 0) {
			dhuart_enable_buf_interrupt(0);
			dhsender_response(id, STATUS_OK, "");
			return 1;
		} else if(!dhuart_init(parse_pins->uart_speed, parse_pins->uart_bits, parse_pins->uart_partity, parse_pins->uart_stopbits)) {
			dhsender_response(id, STATUS_ERROR, "Wrong UART mode");
			return 1;
		}
	}
	return 0;
}

void ICACHE_FLASH_ATTR dhcommands_do(int id, const char *command, const char *params, unsigned int paramslen) {
	gpio_command_params parse_pins;
	ALLOWED_FIELDS fields = 0;
	char *parse_res;
	if( os_strcmp(command, "gpio/write") == 0 ) {
		parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHGPIO_SUITABLE_PINS, 0, AF_SET | AF_CLEAR, &fields);
		if (parse_res)
			dhsender_response(id, STATUS_ERROR, parse_res);
		else if ( (fields & (AF_SET | AF_CLEAR)) == 0)
			dhsender_response(id, STATUS_OK, "Dummy request");
		else if(dhgpio_write(parse_pins.pins_to_set, parse_pins.pins_to_clear))
			dhsender_response(id, STATUS_OK, "");
		else
			dhsender_response(id, STATUS_ERROR, "Unsuitable pin");
	} else if( os_strcmp(command, "gpio/read") == 0 ) {
		int init = 1;
		if(paramslen) {
			parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHGPIO_SUITABLE_PINS, 0, AF_INIT | AF_PULLUP | AF_NOPULLUP, &fields);
			if (parse_res) {
				dhsender_response(id, STATUS_ERROR, parse_res);
				init = 0;
				return;
			} else {
				init = dhgpio_init(parse_pins.pins_to_init, parse_pins.pins_to_pullup, parse_pins.pins_to_nopull);
			}
		}
		if (init) {
			char gpiostatebuff[192]; // 16 gpio inputs with '"10":"1", ' format - 10*16=160 + {,}, null terminated char etc
			dhnotification_gpio_read_to_json(gpiostatebuff, dhgpio_read());
			dhsender_response(id, STATUS_OK, gpiostatebuff);
		} else {
			dhsender_response(id, STATUS_ERROR, "Wrong initialization parameters");
		}
	} else if( os_strcmp(command, "gpio/int") == 0 ) {
		parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHGPIO_SUITABLE_PINS, dhgpio_get_timeout(), AF_DISABLE | AF_RISING | AF_FALLING | AF_BOTH | AF_TIMEOUT, &fields);
		if (parse_res)
			dhsender_response(id, STATUS_ERROR, parse_res);
		else if (fields == 0)
			dhsender_response(id, STATUS_ERROR, "Wrong action");
		else if(parse_pins.timeout < GPIONOTIFICATION_MIN_TIMEOUT_MS || parse_pins.timeout > 0x7fffff)
			dhsender_response(id, STATUS_ERROR, "Timeout is wrong");
		else if(dhgpio_int(parse_pins.pins_to_disable, parse_pins.pins_to_rising, parse_pins.pins_to_falling, \
				parse_pins.pins_to_both, parse_pins.timeout))
			dhsender_response(id, STATUS_OK, "");
		else
			dhsender_response(id, STATUS_ERROR, "Unsuitable pin");
	} else if( os_strcmp(command, "adc/read") == 0) {
		if(paramslen) {
			parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHADC_SUITABLE_PINS, 0, AF_READ, &fields);
			if (parse_res) {
				dhsender_response(id, STATUS_ERROR, parse_res);
				return;
			} else if (parse_pins.pins_to_read != DHADC_SUITABLE_PINS) {
				dhsender_response(id, STATUS_ERROR, "Unknown ADC channel");
				return;
			}
		}
		char adcvalue[48];
		snprintf(adcvalue, sizeof(adcvalue), "{\"0\":\"%f\"}", dhadc_get_value());
		dhsender_response(id, STATUS_OK, adcvalue);
	} else if( os_strcmp(command, "adc/int") == 0) {
		if(paramslen) {
			parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHADC_SUITABLE_PINS, 0, AF_VALUES, &fields);
			if (parse_res) {
				dhsender_response(id, STATUS_ERROR, parse_res);
				return;
			} else if(parse_pins.pin_value_readed != 0x1) {
				dhsender_response(id, STATUS_ERROR, "Wrong adc channel");
				return;
			} else if ((parse_pins.pin_value[0] < ADCNOTIFICATION_MIN_TIMEOUT_MS && parse_pins.pin_value[0] != 0) || parse_pins.pin_value[0] > 0x7fffff) {
				dhsender_response(id, STATUS_ERROR, "Wrong period");
				return;
			} else {
				dhadc_loop(parse_pins.pin_value[0]);
				dhsender_response(id, STATUS_OK, "");
				return;
			}
		}
		dhsender_response(id, STATUS_ERROR, "Wrong parameters");
	} else if( os_strcmp(command, "pwm/control") == 0 ) {
		parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHADC_SUITABLE_PINS, 0, AF_VALUES | AF_PERIOD | AF_COUNT, &fields);
		if (parse_res) {
			dhsender_response(id, STATUS_ERROR, parse_res);
			return;
		}
		if(dhpwm_set_pwm(&parse_pins.pin_value, parse_pins.pin_value_readed, (fields & AF_PERIOD) ? parse_pins.periodus : dhpwm_get_period_us(),  parse_pins.count))
			dhsender_response(id, STATUS_OK, "");
		else
			dhsender_response(id, STATUS_ERROR, "Wrong parameters");
	} else if( os_strcmp(command, "uart/write") == 0 ) {
		parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHADC_SUITABLE_PINS, 0, AF_UARTMODE | AF_DATA, &fields);
		if (parse_res) {
			dhsender_response(id, STATUS_ERROR, parse_res);
			return;
		}
		if(uart_init(id, fields, &parse_pins))
			return;
		dhuart_set_mode(DUM_PER_BUF, dhuart_get_timeout());
		dhuart_send_buf(parse_pins.data, parse_pins.data_len);
		dhsender_response(id, STATUS_OK, "");
	} else if( os_strcmp(command, "uart/int") == 0 ) {
		if(paramslen) {
			parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHADC_SUITABLE_PINS, dhuart_get_timeout(), AF_UARTMODE | AF_TIMEOUT, &fields);
			if (parse_res) {
				dhsender_response(id, STATUS_ERROR, parse_res);
				return;
			}
			if(uart_init(id, fields, &parse_pins))
				return;
		}
		dhuart_set_mode(DUM_PER_BUF, (fields & AF_TIMEOUT) ? parse_pins.timeout : dhuart_get_timeout());
		dhuart_enable_buf_interrupt(1);
		dhsender_response(id, STATUS_OK, "");
	} else if( os_strcmp(command, "uart/terminal") == 0 ) {
		if(paramslen) {
			dhsender_response(id, STATUS_ERROR, "Command does not have parameters");
			return;
		}
		dhterminal_init();
		dhsender_response(id, STATUS_OK, "");
	} else if( os_strcmp(command, "i2c/master/read") == 0 ) {
		parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHADC_SUITABLE_PINS, 0, AF_SDA | AF_SCL | AF_DATA | AF_ADDRESS | AF_COUNT, &fields);
		if (parse_res) {
			dhsender_response(id, STATUS_ERROR, parse_res);
			return;
		}
		if((fields & AF_COUNT) == 0)
			parse_pins.count = 2;
		if(parse_pins.count == 0) {
			dhsender_response(id, STATUS_ERROR, "Can not read 0 bytes");
			return;
		}
		char *res = i2c_init(fields, &parse_pins);
		if(res) {
			dhsender_response(id, STATUS_ERROR, res);
			return;
		}
		if(fields & AF_DATA) {
			res = i2c_status_tochar(dhi2c_write(parse_pins.address, parse_pins.data, parse_pins.data_len, 0));
			if(res) {
				dhsender_response(id, STATUS_ERROR, res);
				return;
			}
		}
		res = i2c_status_tochar(dhi2c_read(parse_pins.address, parse_pins.data, parse_pins.count));
		if (res) {
			dhsender_response(id, STATUS_ERROR, res);
			return;
		}
		char result[INTERFACES_BUF_SIZE * 2];
		res = dhnotification_prepare_data_parameters(result, sizeof(result), parse_pins.data, parse_pins.count);
		if(res) {
			dhsender_response(id, STATUS_ERROR, res);
			return;
		}
		dhsender_response(id, STATUS_OK, result);
	} else if( os_strcmp(command, "i2c/master/write") == 0 ) {
		parse_res = parse_params_pins_set(params, paramslen, &parse_pins, DHADC_SUITABLE_PINS, 0, AF_SDA | AF_SCL | AF_DATA | AF_ADDRESS, &fields);
		if (parse_res) {
			dhsender_response(id, STATUS_ERROR, parse_res);
			return;
		}
		if((fields & AF_DATA) == 0) {
			dhsender_response(id, STATUS_ERROR, "Data not specified");
			return;
		}
		char *res = i2c_init(fields, &parse_pins);
		if(res) {
			dhsender_response(id, STATUS_ERROR, res);
			return;
		}
		res = i2c_status_tochar(dhi2c_write(parse_pins.address, parse_pins.data, parse_pins.data_len, 1));
		if(res)
			dhsender_response(id, STATUS_ERROR, res);
		else
			dhsender_response(id, STATUS_OK, "");
	} else {
		dhsender_response(id, STATUS_ERROR, "Unknown command");
	}
}
