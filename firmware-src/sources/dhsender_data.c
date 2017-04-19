/*
 * dhsender_data.c
 *
 * Copyright 2016 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Helper function for sender data
 *
 */
#include "dhsender_data.h"
#include "dhdata.h"
#include "dhdebug.h"
#include "dhgpio.h"
#include "snprintf.h"

#include <c_types.h>
#include <osapi.h>

void ICACHE_FLASH_ATTR dhsender_data_parse_va(va_list ap,
		REQUEST_DATA_TYPE *data_type, SENDERDATA *data, unsigned int *data_len,
		unsigned int *pin) {
	switch(*data_type) {
		case RDT_CONST_STRING:
			data->string = va_arg(ap, const char *);
			*data_len = sizeof(const char *);
			break;
		case RDT_GPIO:
			data->gpio.caused = va_arg(ap, unsigned int);
			data->gpio.state = va_arg(ap, unsigned int);
			data->gpio.timestamp = va_arg(ap, unsigned int);
			data->gpio.suitable = va_arg(ap, unsigned int);
			*data_len = sizeof(GPIO_DATA);
			break;
		case RDT_FLOAT:
			data->adc = (float)va_arg(ap, double);
			*data_len = sizeof(float);
			break;
		case RDT_SEARCH64:
			*pin = va_arg(ap, unsigned int);
			/* no break */
		case RDT_DATA_WITH_LEN:
		{
			const char *s = va_arg(ap, char *);
			char *d = data->array;
			unsigned int l = va_arg(ap, unsigned int);
			*data_len = l;
			while(l--)
				*d++ = *s++;
		}
			break;
		case RDT_FORMAT_STRING:
			*data_len = vsnprintf(data->array, sizeof(data->array), va_arg(ap, char *), ap);
			break;
		default:
			data->string = "ERROR: Unknown response data type.";
			*data_type = RDT_CONST_STRING;
			dhdebug("ERROR: Unknown response data type %d", data_type);
	}
}

LOCAL unsigned int ICACHE_FLASH_ATTR gpio_state(char *buf,
		unsigned int buflen, unsigned int state, unsigned int suitable) {
	unsigned int len = snprintf(buf, buflen, "{");
	unsigned int i;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		const unsigned int pin = 1 << i;
		const int pinvalue = (state & pin) ? 1 : 0;
		if(suitable & pin) {
			len += snprintf(&buf[len], buflen - len, (i == 0) ? "\"%d\":%d" : ", \"%d\":%d", i, pinvalue);
		}
	}
	return len + snprintf(&buf[len], buflen - len, "}");
}

LOCAL unsigned int ICACHE_FLASH_ATTR gpio_notification(char *buf,
		unsigned int buflen, const GPIO_DATA *data, unsigned int suitable) {
	unsigned int len = snprintf(buf, buflen, "{\"caused\":[");
	unsigned int i;
	int comma = 0;
	for(i = 0; i <= DHGPIO_MAXGPIONUM; i++) {
		const unsigned int pin = 1 << i;
		if((suitable & pin) == 0)
			continue;
		if( pin & data->caused) {
			len += snprintf(&buf[len], buflen - len, comma?", \"%d\"":"\"%d\"", i);
			comma = 1;
		}
	}
	len += snprintf(&buf[len], buflen - len, "], \"state\":");
	len += gpio_state(&buf[len], buflen - len, data->state, suitable);
	return len + snprintf(&buf[len], buflen - len,
			", \"tick\":%u}", data->timestamp);
}

int ICACHE_FLASH_ATTR dhsender_data_to_json(char *buf, unsigned int buf_len,
		int is_notification, REQUEST_DATA_TYPE data_type, SENDERDATA *data,
		unsigned int data_len, unsigned int pin) {
	switch(data_type) {
		case RDT_FORMAT_STRING:
			return snprintf(buf, buf_len, "\"%s\"", data->array);
		case RDT_CONST_STRING:
			return snprintf(buf, buf_len, "\"%s\"", data->string);
		case RDT_DATA_WITH_LEN:
		{
			const unsigned int pos = snprintf(buf, buf_len, "{\"data\":\"");
			const unsigned int res = dhdata_encode(data->array, data_len,
					&buf[pos], buf_len - pos - 3);
			if(res == 0 && data_len != 0) {
				return -1;
			} else {
				return pos + res + snprintf(&buf[pos + res], buf_len - pos,
						"\"}");
			}
		}
		case RDT_FLOAT:
			return snprintf(buf, buf_len, "{\"0\":%f}", data->adc);
		case RDT_GPIO:
			if(is_notification) {
				return gpio_notification(buf, buf_len, &data->gpio, data->gpio.suitable);
			} else {
				return gpio_state(buf, buf_len, data->gpio.state, data->gpio.suitable);
			}
		case RDT_SEARCH64:
		{
			unsigned int i;
			unsigned int len = snprintf(buf, buf_len, "{\"found\":[");
			if(data_len) {
				len += snprintf(&buf[len], buf_len - len, "\"");
				for(i = 0; i < data_len; i++) {
					if(i % 8 == 0 &&  i != 0) {
						len += snprintf(&buf[len], buf_len - len, "\", \"");
					}
					if(len + 2 < buf_len)
						len += byteToHex(data->array[data_len - i - 1], &buf[len]);
				}
				len += snprintf(&buf[len], buf_len - len, "\"");
			}
			return len + snprintf(&buf[len], buf_len - len,
					"], \"pin\":\"%d\"}", pin);
		}
		default:
			dhdebug("ERROR: Unknown data type of request %d", data_type);
	}
	return -1;
}
