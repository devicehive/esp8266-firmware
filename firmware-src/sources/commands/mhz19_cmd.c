/**
 * @file
 * @brief MHZ19 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/mhz19_cmd.h"
#include "devices/mhz19.h"

#include "dhcommand_parser.h"
#include <user_interface.h>


/*
 * dh_handle_devices_mhz19_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_mhz19_read(COMMAND_RESULT *cmd_res, const char *command,
                                                    const char *params, unsigned int params_len)
{
	if (params_len) {
		dh_command_fail(cmd_res, "Command does not have parameters");
		return; // FAILED
	}

	int co2;
	const char *err_msg = mhz19_read(&co2);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
	} else {
		cmd_res->callback(cmd_res->data, DHSTATUS_OK,
				RDT_FORMAT_STRING, "{\"co2\":%d}", co2);
	}
}
