/**
 * @file
 * @brief SPI command handlers.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/spi_cmd.h"
#include "DH/spi.h"
#include "DH/adc.h"

#ifdef DH_COMMANDS_SPI // SPI command handlers
#include "dhcommand_parser.h"
#include <user_interface.h>

/**
 * @brief SPI initialization helper.
 * @return Non-zero if SPI was initialized. Zero otherwise.
 */
static int ICACHE_FLASH_ATTR spi_init(COMMAND_RESULT *cmd_res, ALLOWED_FIELDS fields, const gpio_command_params *params)
{
	if (fields & AF_CS) {
		if (!!dh_spi_set_cs_pin(params->CS)) {
			dh_command_fail(cmd_res, "Wrong CS pin");
			return 1; // FAILED
		}
	}

	if(fields & AF_SPIMODE) {
		if (!!dh_spi_set_mode((DHSpiMode)params->spi_mode)) {
			dh_command_fail(cmd_res, "Wrong SPI mode");
			return 1; // FAILED
		}
	}

	return 0; // continue
}


/*
 * do_spi_master_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_spi_master_read(COMMAND_RESULT *cmd_res, const char *command,
                                                 const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_CS | AF_SPIMODE | AF_DATA | AF_COUNT, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return;
	}

	if (!(fields & AF_COUNT))
		info.count = 2;
	if (info.count == 0 || info.count > INTERFACES_BUF_SIZE) {
		dh_command_fail(cmd_res, "Wrong read size");
		return;
	}

	if (spi_init(cmd_res, fields, &info))
		return;

	if (fields & AF_DATA)
		dh_spi_write(info.data, info.data_len, 0);
	dh_spi_read(info.data, info.count);

	dh_command_done_buf(cmd_res, info.data, info.count);
}


/*
 * do_spi_master_write() implementation.
 */
void ICACHE_FLASH_ATTR dh_handle_spi_master_write(COMMAND_RESULT *cmd_res, const char *command,
                                                  const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_CS | AF_SPIMODE | AF_DATA, &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return;
	}

	if (spi_init(cmd_res, fields, &info))
		return;

	if (!(fields & AF_DATA)) {
		dh_command_fail(cmd_res, "Data not specified");
		return;
	}

	dh_spi_write(info.data, info.data_len, 1);
	dh_command_done(cmd_res, "");
}

#endif /* DH_COMMANDS_SPI */
