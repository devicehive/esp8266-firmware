/**
 * @file
 * @brief SPI bus implementation for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "DH/spi.h"
#include "DH/gpio.h"
#include "DH/adc.h"

#include <eagle_soc.h>
#include <osapi.h>
#include <gpio.h>
#include <ets_forward.h>

#define FUNC_HSPI               2
#define SPI_BASE                0x60000100 // HSPI
#define SPI_CTRL2               0x60000114
#define SPI_CLOCK               0x60000118
#define SPI_USER                0x6000011C
#define SPI_USER1               0x60000120
#define SPI_W0                  0x60000140
#define SPI_USR_MISO            BIT(28)
#define SPI_USR_MOSI            BIT(27)
#define SPI_CLKDIV_PRE          0x00001FFF
#define SPI_CLKDIV_PRE_S        18
#define SPI_CLKCNT_N            0x0000003F
#define SPI_CLKCNT_N_S          12
#define SPI_CLKCNT_H            0x0000003F
#define SPI_CLKCNT_H_S          6
#define SPI_CLKCNT_L            0x0000003F
#define SPI_CLKCNT_L_S          0
#define SPI_CK_OUT_HIGH_MODE    0x0000000F
#define SPI_CK_OUT_HIGH_MODE_S  12
#define SPI_CK_OUT_LOW_MODE     0x0000000F
#define SPI_CK_OUT_LOW_MODE_S   8
#define SPI_CK_OUT_EDGE         BIT(7)
#define SPI_CK_I_EDGE           BIT(6)
#define SPI_USR_MOSI_BITLEN     0x000001FF
#define SPI_USR_MOSI_BITLEN_S   17
#define SPI_USR_MISO_BITLEN     0x000001FF
#define SPI_USR_MISO_BITLEN_S   8
#define SPI_USR                 BIT(18)
#define SPI_CS_DELAY_US         2

// module variables
static unsigned int mSPICSPin = DH_SPI_NO_CS;
static DHSpiMode mSPIMode = DH_SPI_CPOL0CPHA0;


/**
 * @brief Enable CS.
 */
static void ICACHE_FLASH_ATTR spi_enable_cs(void)
{
	if (mSPICSPin == DH_SPI_NO_CS)
		return; // do nothing

	dh_gpio_write(0, DH_GPIO_PIN(mSPICSPin));
	os_delay_us(SPI_CS_DELAY_US);
}


/**
 * @brief Disable CS.
 */
static void ICACHE_FLASH_ATTR spi_disable_cs(void)
{
	if (mSPICSPin == DH_SPI_NO_CS)
		return; // do nothing

	dh_gpio_write(DH_GPIO_PIN(mSPICSPin), 0);
	os_delay_us(SPI_CS_DELAY_US);
}


/**
 * @brief Re-initialize SPI module.
 */
static void ICACHE_FLASH_ATTR spi_reinit(void)
{
	const DHGpioPinMask pins = DH_GPIO_PIN(12) | DH_GPIO_PIN(13) | DH_GPIO_PIN(14);
	dh_gpio_open_drain(0, pins);
	dh_gpio_pull_up(0, pins);
	gpio_output_set(0, 0, 0, pins);

	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_HSPI);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_HSPI);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_HSPI);
	WRITE_PERI_REG(SPI_USER, 0);
	WRITE_PERI_REG(SPI_CLOCK, // 1 MHz
			((15 & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S)
			| ((4 & SPI_CLKCNT_N) << SPI_CLKCNT_N_S)
			| ((1 & SPI_CLKCNT_H) << SPI_CLKCNT_H_S)
			| ((3 & SPI_CLKCNT_L) << SPI_CLKCNT_L_S));
	if (mSPIMode & 0x1) { // CPHA
		SET_PERI_REG_MASK(SPI_USER, SPI_CK_OUT_EDGE);
		CLEAR_PERI_REG_MASK(SPI_USER, SPI_CK_I_EDGE);
	} else {
		SET_PERI_REG_MASK(SPI_USER, SPI_CK_I_EDGE);
		CLEAR_PERI_REG_MASK(SPI_USER, SPI_CK_OUT_EDGE);
	}
	if (mSPIMode & 0x2) { // CPOL
		SET_PERI_REG_MASK(SPI_CTRL2, SPI_CK_OUT_HIGH_MODE << SPI_CK_OUT_HIGH_MODE_S);
		CLEAR_PERI_REG_MASK(SPI_CTRL2, SPI_CK_OUT_LOW_MODE << SPI_CK_OUT_LOW_MODE_S);
	} else {
		SET_PERI_REG_MASK(SPI_CTRL2, SPI_CK_OUT_LOW_MODE << SPI_CK_OUT_LOW_MODE_S);
		CLEAR_PERI_REG_MASK(SPI_CTRL2, SPI_CK_OUT_HIGH_MODE << SPI_CK_OUT_LOW_MODE_S);
	}

	WRITE_PERI_REG(SPI_USER1,
			((7 & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S)
			| ((7 & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S));

	spi_enable_cs();
}


/*
 * dh_spi_set_mode() implementation.
 */
int ICACHE_FLASH_ATTR dh_spi_set_mode(DHSpiMode mode)
{
	switch (mode) {
	case DH_SPI_CPOL0CPHA0:
	case DH_SPI_CPOL0CPHA1:
	case DH_SPI_CPOL1CPHA0:
	case DH_SPI_CPOL1CPHA1:
		break; // OK

	default:
		return -1; // bad mode
	}

	mSPIMode = mode;
	return 0; // OK
}


/*
 * dh_spi_set_cs_pin() implementation.
 */
int ICACHE_FLASH_ATTR dh_spi_set_cs_pin(unsigned int pin)
{
	if (pin != DH_SPI_NO_CS) {
		if (pin >= DH_GPIO_PIN_COUNT)
			return -1; // bad pin number
		if (!(DH_GPIO_PIN(pin) & DH_GPIO_SUITABLE_PINS))
			return -1; // unsuitable pin
		if (pin == 12 || pin == 13 || pin == 14)
			return -1; // protected pins
	}

	mSPICSPin = pin;
	return 0; // OK
}


/*
 * dh_spi_write() implementation.
 */
void ICACHE_FLASH_ATTR dh_spi_write(const void *buf_, size_t len, int disable_cs)
{
	spi_reinit();

	CLEAR_PERI_REG_MASK(SPI_USER, SPI_USR_MISO);
	SET_PERI_REG_MASK(SPI_USER, SPI_USR_MOSI);

	const uint8_t *buf = (const uint8_t*)buf_;
	while (len--) {
		while (READ_PERI_REG(SPI_BASE) & SPI_USR)
			;
		WRITE_PERI_REG(SPI_W0, *buf++);
		SET_PERI_REG_MASK(SPI_BASE, SPI_USR);
	}

	if (disable_cs)
		spi_disable_cs();
}


/*
 * dh_spi_read() implementation.
 */
void ICACHE_FLASH_ATTR dh_spi_read(void *buf_, size_t len)
{
	spi_reinit();

	CLEAR_PERI_REG_MASK(SPI_USER, SPI_USR_MOSI);
	SET_PERI_REG_MASK(SPI_USER, SPI_USR_MISO);
	while (READ_PERI_REG(SPI_BASE) & SPI_USR)
		;

	uint8_t *buf = (uint8_t*)buf_;
	while (len--) {
		SET_PERI_REG_MASK(SPI_BASE, SPI_USR);
		while (READ_PERI_REG(SPI_BASE) & SPI_USR)
			;
		*buf++ = READ_PERI_REG(SPI_W0) & 0xff;
	}

	spi_disable_cs();
}



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

	cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_DATA_WITH_LEN, info.data, info.count);
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
