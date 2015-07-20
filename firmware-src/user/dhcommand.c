/*
 * dhcommand.cpp
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for parsing server command
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <json/jsonparse.h>
#include "dhcommand.h"
#include "dhsender.h"
#include "dhgpio.h"
#include "dhadc.h"
#include "dhnotification.h"
#include "snprintf.h"

typedef struct {
	unsigned int pins_to_set;
	unsigned int pins_to_clear;
	unsigned int pins_to_init;
	unsigned int pins_to_pullup;
	unsigned int pins_to_nopull;
	unsigned int pins_to_disable;
	unsigned int pins_to_rising;
	unsigned int pins_to_falling;
	unsigned int pins_to_both;
	unsigned int pins_to_low;
	unsigned int pins_to_high;
	unsigned int pins_to_read;
} gpio_command_params;

int ICACHE_FLASH_ATTR strToInt(const char *ptr, int *result){
	unsigned char d;
	int res = 0;
	int sign = 1;
	if(*ptr == '-') {
		sign = -1;
		ptr++;
	}
	if (*ptr >= '0' && *ptr <= '9') {
		while ( (d = *ptr - 0x30) < 10) {
			ptr++;
			res = res*10 + d;
		}
		*result = res * sign;
		return 1;
	} else {
		return 0;
	}
}

LOCAL char * ICACHE_FLASH_ATTR parse_params_pins_set(struct jsonparse_state *jparser, gpio_command_params *out, unsigned int all) {
	int type;
	int pin;
	unsigned int pinmask;
	os_memset(out, 0, sizeof(gpio_command_params));
	while ((type = jsonparse_next(jparser)) != JSON_TYPE_ERROR) {
		if (type == JSON_TYPE_PAIR_NAME) {
			if(jsonparse_strcmp_value(jparser, "all") == 0) {
				pinmask = all;
			} else {
				const int res = strToInt(&jparser->json[jparser->vstart], &pin);
				if(!res || pin < 0 || pin > 31)
					return "Wrong pin";
				pinmask =  (1 << pin);
			}
			jsonparse_next(jparser);
			if(jsonparse_next(jparser) != JSON_TYPE_ERROR) {
				if(jsonparse_strcmp_value(jparser, "1") == 0)
					out->pins_to_set |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "0") == 0)
					out->pins_to_clear |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "x") == 0)
					continue;
				else if(jsonparse_strcmp_value(jparser, "init") == 0)
					out->pins_to_init |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "pullup") == 0)
					out->pins_to_pullup |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "nopull") == 0)
					out->pins_to_nopull |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "disable") == 0)
					out->pins_to_disable |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "rising") == 0)
					out->pins_to_rising |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "falling") == 0)
					out->pins_to_falling |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "both") == 0)
					out->pins_to_both |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "low") == 0)
					out->pins_to_low |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "high") == 0)
					out->pins_to_high |= pinmask;
				else if(jsonparse_strcmp_value(jparser, "read") == 0)
					out->pins_to_read |= pinmask;
				else return "Unsupported action";
			}
		}
	}
	return NULL;
}

void ICACHE_FLASH_ATTR dhcommand_do(int id, const char *command, const char *params, unsigned int paramslen) {
	struct jsonparse_state jparser;
	gpio_command_params parse_pins;
	char *parse_res;
	int interuption;
	if(paramslen)
		jsonparse_setup(&jparser, params, paramslen);
	if( os_strcmp(command, "gpio/write") == 0 ) {
		if(paramslen == 0)
			dhsender_response(id, STATUS_ERROR, "No parameters specified");
		parse_res = parse_params_pins_set(&jparser, &parse_pins, DHGPIO_SUITABLE_PINS);
		if (parse_res)
			dhsender_response(id, STATUS_ERROR, parse_res);
		else if (parse_pins.pins_to_set == 0 && parse_pins.pins_to_clear == 0)
			dhsender_response(id, STATUS_ERROR, "Wrong action");
		else if(dhgpio_write(parse_pins.pins_to_set, parse_pins.pins_to_clear))
			dhsender_response(id, STATUS_OK, "");
		else
			dhsender_response(id, STATUS_ERROR, "Unsuitable pin");
	} else if( os_strcmp(command, "gpio/read") == 0 ) {
		int init = 1;
		if(paramslen) {
			parse_res = parse_params_pins_set(&jparser, &parse_pins, DHGPIO_SUITABLE_PINS);
			if (parse_res) {
				dhsender_response(id, STATUS_ERROR, parse_res);
				init = 0;
			} else {
				init = dhgpio_init(parse_pins.pins_to_init, parse_pins.pins_to_pullup, parse_pins.pins_to_nopull);
			}
		}
		if (init) {
			char gpiostatebuff[192]; // 16 gpio inputs with '"10":"1", ' format - 10*16=128 + {,}, null terminated char etc
			dhgpio_read_to_json(gpiostatebuff, dhgpio_read());
			dhsender_response(id, STATUS_OK, gpiostatebuff);
		} else {
			dhsender_response(id, STATUS_ERROR, "Wrong initialization parameters");
		}
	} else if( os_strcmp(command, "gpio/int") == 0 ) {
		if(paramslen == 0)
			dhsender_response(id, STATUS_ERROR, "No parameters specified");
		parse_res = parse_params_pins_set(&jparser, &parse_pins, DHGPIO_SUITABLE_PINS);
		if (parse_res)
			dhsender_response(id, STATUS_ERROR, parse_res);
		else if (parse_pins.pins_to_disable == 0 && parse_pins.pins_to_rising == 0 && parse_pins.pins_to_falling == 0 \
				&& parse_pins.pins_to_both == 0 && parse_pins.pins_to_low == 0 && parse_pins.pins_to_high == 0)
			dhsender_response(id, STATUS_ERROR, "Wrong action");
		else if(dhgpio_int(parse_pins.pins_to_disable, parse_pins.pins_to_rising, parse_pins.pins_to_falling, \
				parse_pins.pins_to_both, parse_pins.pins_to_low, parse_pins.pins_to_high))
			dhsender_response(id, STATUS_OK, "");
		else
			dhsender_response(id, STATUS_ERROR, "Unsuitable pin");
	} else if( os_strcmp(command, "adc/read") == 0) {
		if(paramslen) {
			parse_res = parse_params_pins_set(&jparser, &parse_pins, DHADC_SUITABLE_PINS);
			if (parse_res) {
				dhsender_response(id, STATUS_ERROR, parse_res);
				return;
			} else if (parse_pins.pins_to_read != DHADC_SUITABLE_PINS) {
				dhsender_response(id, STATUS_ERROR, "Unknown ADC channel");
				return;
			}
		}
		char adcvalue[48];
		snprintf(adcvalue, sizeof(adcvalue), "{\"0\":\"%f\"}", dhadc_get_value()/1024.0f);
		dhsender_response(id, STATUS_OK, adcvalue);
	} else if( os_strcmp(command, "adc/int") == 0) {
		if(paramslen) {
			int type;
			while ((type = jsonparse_next(&jparser)) != JSON_TYPE_ERROR) {
				if (type == JSON_TYPE_PAIR_NAME) {
					if(jsonparse_strcmp_value(&jparser, "0") == 0) {
						jsonparse_next(&jparser);
						if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
							int period;
							if(jsonparse_strcmp_value(&jparser, "disable") == 0) {
								period = 0;
							} else {
								const int res = strToInt(&jparser.json[jparser.vstart], &period);
								if(!res || period < 50 || period > 0x7fffff) {
									dhsender_response(id, STATUS_ERROR, "Wrong period");
									return;
								}
							}
							dhnotification_adc(period);
							dhsender_response(id, STATUS_OK, "");
							return;
						}
					}
				}
			}
		}
		dhsender_response(id, STATUS_ERROR, "Wrong parameters");
	} else {
		dhsender_response(id, STATUS_ERROR, "Unknown command");
	}
}
