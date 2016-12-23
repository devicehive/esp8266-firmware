/*
 * dhcommand_parser.cpp
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Module for parsing server commands
 *
 */

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <json/jsonparse.h>
#include "dhcommand_parser.h"
#include "dhutils.h"
#include "dhdata.h"

static char * const UNEXPECTED = "Unexpected parameter";
static char * const NONINTEGER = "Non integer value";
static char * const NONFLOAT = "Non float value";

LOCAL int ICACHE_FLASH_ATTR strcmp_value(struct jsonparse_state *jparser, const char *str) {
	const char *json = &jparser->json[jparser->vstart];
	int len = 0;
	while(*str && *json) {
		if(*str != *json)
			return *json - *str;
		str++;
		json++;
		len++;
	}
	if(len != jparser->vlen) {
		return 1;
	}
	return (*str == 0) ? 0 : (*json - *str);
}

LOCAL void ICACHE_FLASH_ATTR load_defaults(gpio_command_params *out, unsigned int timeout) {
	os_memset(out, 0, sizeof(gpio_command_params));
	out->uart_speed = 115200;
	out->uart_bits = 8;
	out->uart_partity = 'N';
	out->uart_stopbits = 1;
	out->timeout = timeout;
	out->data_len = 0;
}

LOCAL char * ICACHE_FLASH_ATTR readUIntField(struct jsonparse_state *jparser, ALLOWED_FIELDS field, uint32_t *out, ALLOWED_FIELDS fields, ALLOWED_FIELDS *readedfields, unsigned int x) {
	if((fields & field) == 0)
		return UNEXPECTED;
	int value;
	jsonparse_next(jparser);
	if(jsonparse_next(jparser) != JSON_TYPE_ERROR) {
		if(x && jparser->vlen == 1 && jparser->json[jparser->vstart] == 'x' && (field & AF_CS)) {
			*out = x;
			*readedfields |= field;
			return 0;
		}
		const int res = strToUInt(&jparser->json[jparser->vstart], &value);
		if(!res)
			return NONINTEGER;
		*out = value;
		*readedfields |= field;
	}
	return 0;
}

LOCAL char * ICACHE_FLASH_ATTR readFloatField(struct jsonparse_state *jparser, ALLOWED_FIELDS field, float *out, ALLOWED_FIELDS fields, ALLOWED_FIELDS *readedfields) {
	if((fields & field) == 0)
		return UNEXPECTED;
	float value;
	jsonparse_next(jparser);
	if(jsonparse_next(jparser) != JSON_TYPE_ERROR) {
		const int res = strToFloat(&jparser->json[jparser->vstart], &value);
		if(!res)
			return NONFLOAT;
		*out = value;
		*readedfields |= field;
	}
	return 0;
}

char * ICACHE_FLASH_ATTR parse_params_pins_set(const char *params, unsigned int paramslen, gpio_command_params *out, unsigned int all, unsigned int timeout, ALLOWED_FIELDS fields, ALLOWED_FIELDS *readedfields) {
	struct jsonparse_state jparser;
	if(paramslen)
		jsonparse_setup(&jparser, params, paramslen);
	else if(fields)
		return "No parameters specified";
	int type;
	int pinnum;
	unsigned int pins_found = 0;
	unsigned int pinmask;
	*readedfields = 0;
	load_defaults(out, timeout);
	while (jparser.pos < jparser.len) {
		type = jsonparse_next(&jparser);
		if (type == JSON_TYPE_PAIR_NAME) {
			if(strcmp_value(&jparser, "mode") == 0) {
				if((fields & AF_UARTMODE) == 0 && (fields & AF_SPIMODE) == 0)
					return UNEXPECTED;
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
					int val;
					if(strcmp_value(&jparser, "disable") == 0 && (fields & AF_UARTMODE)) {
						*readedfields |= AF_UARTMODE;
						out->uart_speed = 0;
						continue;
					}
					int res = strToUInt(&jparser.json[jparser.vstart], &val);
					if(!res)
						return "Wrong mode integer value";
					if(fields & AF_UARTMODE) {
						*readedfields |= AF_UARTMODE;
						out->uart_speed = val;
						if(res < jparser.vlen && jparser.json[jparser.vstart + res] == ' ') {
							while(res < jparser.vlen && jparser.json[jparser.vstart + res] == ' ')
								res++;
							if(res + 3 == jparser.vlen) {
								const unsigned char b =  jparser.json[jparser.vstart + res] - '0';
								const unsigned char p =  jparser.json[jparser.vstart + res + 1];
								const unsigned char s =  jparser.json[jparser.vstart + res + 2] - '0';
								if(b < 10 && s < 10) {
									out->uart_bits = b;
									out->uart_partity = p;
									out->uart_stopbits = s;
								} else {
									return "Wrong mode framing";
								}
							}
						}
					}
					if(fields & AF_SPIMODE) {
						*readedfields |= AF_SPIMODE;
						out->spi_mode = val;
					}
				}
				continue;
			} else if(strcmp_value(&jparser, "count") == 0) {
				char * res = readUIntField(&jparser, AF_COUNT, &out->count, fields, readedfields, 0);
				if(res)
					return res;
				continue;
			} else if(strcmp_value(&jparser, "timeout") == 0) {
				char * res = readUIntField(&jparser, AF_TIMEOUT, &out->timeout, fields, readedfields, 0);
				if(res)
					return res;
				continue;
			} else if(strcmp_value(&jparser, "frequency") == 0) {
				if((fields & AF_PERIOD) == 0)
					return UNEXPECTED;
				float frequncy;
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
					const int res = strToFloat(&jparser.json[jparser.vstart], &frequncy);
					if(!res)
						return "Wrong frequency float value";
					if(frequncy < 0.000499999f)
						out->periodus = 2000004000;
					else
						out->periodus = 1000000.0f / frequncy;
					*readedfields |= AF_PERIOD;
				}
				continue;
			} else if(strcmp_value(&jparser, "address") == 0) {
				if((fields & AF_ADDRESS) == 0)
					return UNEXPECTED;
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
					char c;
					int p;
					if(jparser.json[jparser.vstart] == '0' && jparser.json[jparser.vstart + 1] == 'x')
						p = 2;
					else
						p = 0;
					const int res = hexToByte(&jparser.json[jparser.vstart + p], &c);
					if(res != (jparser.vlen - p))
						return "Address is wrong";
					out->address = c;
					*readedfields |= AF_ADDRESS;
				}
				continue;
			} else if(strcmp_value(&jparser, "SDA") == 0) {
				char * res = readUIntField(&jparser, AF_SDA, &out->SDA, fields, readedfields, 0);
				if(res)
					return res;
				continue;
			} else if(strcmp_value(&jparser, "SCL") == 0) {
				char * res = readUIntField(&jparser, AF_SCL, &out->SCL, fields, readedfields, 0);
				if(res)
					return res;
				continue;
			} else if(strcmp_value(&jparser, "CS") == 0) {
				char * res = readUIntField(&jparser, AF_CS, &out->CS, fields, readedfields,  ~(uint32_t)0U);
				if(res)
					return res;
				continue;
			} else if(strcmp_value(&jparser, "pin") == 0) {
				char * res = readUIntField(&jparser, AF_PIN, &out->pin, fields, readedfields, 0);
				if(res)
					return res;
				continue;
			} else if(strcmp_value(&jparser, "ref") == 0) {
				char * res = readFloatField(&jparser, AF_REF, &out->ref, fields, readedfields);
				if(res)
					return res;
				continue;
			} else if(strcmp_value(&jparser, "data") == 0) {
				if((fields & AF_DATA) == 0 || out->data_len)
					return UNEXPECTED;
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
					out->data_len = dhdata_decode(&jparser.json[jparser.vstart], jparser.vlen, out->data, sizeof(out->data));
					if(out->data_len == 0)
						return "Data is broken";
					*readedfields |= AF_DATA;
				}
				continue;
			} else if(strcmp_value(&jparser, "key") == 0) {
				if((fields & AF_KEY) == 0 || out->data_len)
					return UNEXPECTED;
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
					out->storage.key.key_len = dhdata_decode(&jparser.json[jparser.vstart], jparser.vlen, out->storage.key.key_data, sizeof(out->storage.key.key_data));
					if(out->storage.key.key_len == 0)
						return "Key is broken";
					*readedfields |= AF_KEY;
				}
				continue;
			} else if(strcmp_value(&jparser, "text") == 0) {
				if((fields & AF_TEXT_DATA) == 0 || out->data_len)
					return UNEXPECTED;
				jsonparse_next(&jparser);
				if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
					if (jparser.vlen > sizeof(out->data) - 1)
						return "Text is too long";
					os_memcpy(out->data, &jparser.json[jparser.vstart], jparser.vlen);
					out->data[jparser.vlen] = 0;
					out->data_len = jparser.vlen;
					*readedfields |= AF_TEXT_DATA;
				}
				continue;
			} else if(strcmp_value(&jparser, "all") == 0) {
				if(pins_found)
					return "Wrong argument";
				pins_found = ~(unsigned int)0;
				pinmask = all;
				pinnum = -1;
			} else {
				const int res = strToUInt(&jparser.json[jparser.vstart], &pinnum);
				if(!res || pinnum < 0 || pinnum > DHGPIO_MAXGPIONUM || (pins_found & (1 << pinnum)))
					return "Wrong argument";
				pins_found |= (1 << pinnum);
				pinmask =  (1 << pinnum);
			}
			jsonparse_next(&jparser);
			if(jsonparse_next(&jparser) != JSON_TYPE_ERROR) {
				if(strcmp_value(&jparser, "x") == 0)
					continue;
				else if(strcmp_value(&jparser, "init") == 0) {
					if((fields & AF_INIT) == 0)
						return UNEXPECTED;
					out->pins_to_init |= pinmask;
					*readedfields |= AF_INIT;
				} else if(strcmp_value(&jparser, "pullup") == 0) {
					if((fields & AF_PULLUP) == 0)
						return UNEXPECTED;
					out->pins_to_pullup |= pinmask;
					*readedfields |= AF_PULLUP;
				} else if(strcmp_value(&jparser, "nopull") == 0) {
					if((fields & AF_NOPULLUP) == 0)
						return UNEXPECTED;
					out->pins_to_nopull |= pinmask;
					*readedfields |= AF_NOPULLUP;
				} else if(strcmp_value(&jparser, "disable") == 0) {
					if((fields & AF_VALUES) == 0 && (fields & AF_DISABLE) == 0) {
						return UNEXPECTED;
					}
					if (fields & AF_VALUES) {
						int i;
						if(pinnum > 0 )
							out->storage.uint_values[pinnum] = 0;
						else for(i = 0; i <= DHGPIO_MAXGPIONUM; i++)
							out->storage.uint_values[i] = 0;
						out->pin_value_readed |= pinmask;
						*readedfields |= AF_VALUES;
					}
					if(fields & AF_DISABLE) {
						out->pins_to_disable |= pinmask;
						*readedfields |= AF_DISABLE;
					}
				} else if(strcmp_value(&jparser, "rising") == 0) {
					if((fields & AF_RISING)== 0)
						return UNEXPECTED;
					out->pins_to_rising |= pinmask;
					*readedfields |= AF_RISING;
				} else if(strcmp_value(&jparser, "falling") == 0) {
					if((fields & AF_FALLING) == 0)
						return UNEXPECTED;
					out->pins_to_falling |= pinmask;
					*readedfields |= AF_FALLING;
				} else if(strcmp_value(&jparser, "both") == 0) {
					if((fields & AF_BOTH) == 0)
						return UNEXPECTED;
					out->pins_to_both |= pinmask;
					*readedfields |= AF_BOTH;
				} else if(strcmp_value(&jparser, "read") == 0) {
					if((fields & AF_READ) == 0)
						return UNEXPECTED;
					out->pins_to_read |= pinmask;
					*readedfields |= AF_READ;
				} else if(strcmp_value(&jparser, "presence") == 0) {
					if((fields & AF_PRESENCE) == 0)
						return UNEXPECTED;
					out->pins_to_presence |= pinmask;
					*readedfields |= AF_PRESENCE;
				} else if((fields & AF_FLOATVALUES)) { // should be right under AF_VALUES
					float value;
					int i;
					if(!strToFloat(&jparser.json[jparser.vstart], &value))
						return NONFLOAT;
					if(pinnum > 0 )
						out->storage.float_values[pinnum] = value;
					else for(i = 0; i <= DHGPIO_MAXGPIONUM; i++)
						out->storage.float_values[i] = value;
					out->pin_value_readed |= pinmask;
					*readedfields |= AF_FLOATVALUES;
				} else if((fields & AF_VALUES)) { // BE CAREFULL, all digits values have to be under this if
					int value, i;
					if(!strToUInt(&jparser.json[jparser.vstart], &value))
						return NONINTEGER;
					if(pinnum > 0 )
						out->storage.uint_values[pinnum] = value;
					else for(i = 0; i <= DHGPIO_MAXGPIONUM; i++)
						out->storage.uint_values[i] = value;
					out->pin_value_readed |= pinmask;
					*readedfields |= AF_VALUES;
					if(value == 1 && (fields & AF_SET)) {
						out->pins_to_set |= pinmask;
						*readedfields |= AF_SET;
					} else if(value == 0 && (fields & AF_CLEAR)) {
						out->pins_to_clear |= pinmask;
						*readedfields |= AF_CLEAR;
					}
				} else if(strcmp_value(&jparser, "1") == 0) {
					if((fields & AF_SET) == 0)
						return UNEXPECTED;
					out->pins_to_set |= pinmask;
					*readedfields |= AF_SET;
				} else if(strcmp_value(&jparser, "0") == 0) {
					if((fields & AF_CLEAR) == 0)
						return UNEXPECTED;
					out->pins_to_clear |= pinmask;
					*readedfields |= AF_CLEAR;
				} else {
					return "Unsupported action";
				}
			}
		} else if(type == JSON_TYPE_ERROR) {
			return "Broken json";
		}
	}
	return NULL;
}
