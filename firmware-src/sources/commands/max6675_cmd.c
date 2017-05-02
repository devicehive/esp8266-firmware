/**
 * @file
 * @brief MAX6675 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/max6675_cmd.h"
#include "devices/max6675.h"
#include "DH/spi.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>

#if defined(DH_COMMANDS_MAX6675) && defined(DH_DEVICE_MAX6675)

/*
 * dh_handle_devices_max6675_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_max6675_read(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	if (params_len) {
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0, AF_CS, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
	}

	float temperature;
	const char* err_msg = max6675_read((fields & AF_CS) ? info.CS : DH_SPI_NO_PIN, &temperature);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING,
				"{\"temperature\":%f}", temperature);
	}
}

#endif /* DH_COMMANDS_MAX6675 && DH_DEVICE_MAX6675 */
