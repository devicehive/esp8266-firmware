/*
 * dhspi.c
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: SPI implementation
 *
 */

#include <c_types.h>
#include <eagle_soc.h>
#include <osapi.h>
#include "dhspi.h"
#include "dhgpio.h"

#define FUNC_HSPI 2
#define SPI_BASE				0x60000100 // HSPI
#define SPI_CTRL2				0x60000114
#define SPI_CLOCK				0x60000118
#define SPI_USER				0x6000011C
#define SPI_USER1				0x60000120
#define SPI_W0					0x60000140
#define SPI_USR_MISO			(1 << 28)
#define SPI_USR_MOSI			(1 << 27)
#define SPI_CLKDIV_PRE			0x00001FFF
#define SPI_CLKDIV_PRE_S		18
#define SPI_CLKCNT_N 			0x0000003F
#define SPI_CLKCNT_N_S 			12
#define SPI_CLKCNT_H 			0x0000003F
#define SPI_CLKCNT_H_S 			6
#define SPI_CLKCNT_L 			0x0000003F
#define SPI_CLKCNT_L_S 			0
#define SPI_CK_OUT_HIGH_MODE	0x0000000F
#define SPI_CK_OUT_HIGH_MODE_S	12
#define SPI_CK_OUT_LOW_MODE		0x0000000F
#define SPI_CK_OUT_LOW_MODE_S	8
#define SPI_CK_OUT_EDGE			(1 << 7)
#define SPI_CK_I_EDGE			(1 << 6)
#define SPI_USR_MOSI_BITLEN		0x000001FF
#define SPI_USR_MOSI_BITLEN_S	17
#define SPI_USR_MISO_BITLEN		0x000001FF
#define SPI_USR_MISO_BITLEN_S	8
#define SPI_USR					(1 << 18)

#define DHSPI_CS_DELAY_US 2

LOCAL unsigned int mSPICSPin = DHSPI_NOCS;
LOCAL unsigned char mSPIMode = 0;

LOCAL void ICACHE_FLASH_ATTR dhspi_cs_enable() {
	if(mSPICSPin == DHSPI_NOCS)
		return;
	dhgpio_write(0, (1 << mSPICSPin));
	os_delay_us(DHSPI_CS_DELAY_US);
}

LOCAL void ICACHE_FLASH_ATTR dhspi_cs_disable() {
	if(mSPICSPin == DHSPI_NOCS)
		return;
	dhgpio_write((1 << mSPICSPin), 0);
	os_delay_us(DHSPI_CS_DELAY_US);
}

LOCAL void ICACHE_FLASH_ATTR dhspi_reinit() {
	const unsigned int spipins = (1 << 12) | (1 << 13) | (1 << 14);
	dhgpio_open_drain(0, spipins);
	dhgpio_pull(0, spipins);
	gpio_output_set(0, 0, 0, spipins);

	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_HSPI);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_HSPI);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_HSPI);
	WRITE_PERI_REG(SPI_USER, 0);
	WRITE_PERI_REG(
			SPI_CLOCK, // 1 MHz
			((15 & SPI_CLKDIV_PRE) << SPI_CLKDIV_PRE_S) | ((4 & SPI_CLKCNT_N) << SPI_CLKCNT_N_S) | ((1 & SPI_CLKCNT_H) << SPI_CLKCNT_H_S) | ((3 & SPI_CLKCNT_L) << SPI_CLKCNT_L_S));
	if(mSPIMode & 0x1) { // CPHA
		SET_PERI_REG_MASK(SPI_USER, SPI_CK_OUT_EDGE);
		CLEAR_PERI_REG_MASK(SPI_USER, SPI_CK_I_EDGE);
	} else {
		SET_PERI_REG_MASK(SPI_USER, SPI_CK_I_EDGE);
		CLEAR_PERI_REG_MASK(SPI_USER, SPI_CK_OUT_EDGE);
	}
	if(mSPIMode & 0x2) { // CPOL
		SET_PERI_REG_MASK(SPI_CTRL2,
				SPI_CK_OUT_HIGH_MODE << SPI_CK_OUT_HIGH_MODE_S);
		CLEAR_PERI_REG_MASK(SPI_CTRL2,
				SPI_CK_OUT_LOW_MODE << SPI_CK_OUT_LOW_MODE_S);
	} else {
		SET_PERI_REG_MASK(SPI_CTRL2,
				SPI_CK_OUT_LOW_MODE << SPI_CK_OUT_LOW_MODE_S);
		CLEAR_PERI_REG_MASK(SPI_CTRL2,
				SPI_CK_OUT_HIGH_MODE << SPI_CK_OUT_LOW_MODE_S);
	}

	WRITE_PERI_REG(SPI_USER1,
			((7 & SPI_USR_MOSI_BITLEN) << SPI_USR_MOSI_BITLEN_S) | ((7 & SPI_USR_MISO_BITLEN) << SPI_USR_MISO_BITLEN_S));

	dhspi_cs_enable();
}

int ICACHE_FLASH_ATTR dhspi_set_mode(unsigned int mode) {
	if(mode > 3)
		return 0;
	mSPIMode = mode;
	return 1;
}

int ICACHE_FLASH_ATTR dhspi_set_cs_pin(unsigned int cs_pin) {
	if(cs_pin != DHSPI_NOCS
			&& (cs_pin == 12 || cs_pin == 13 || cs_pin == 14
					|| cs_pin > DHGPIO_MAXGPIONUM
					|| ((1 << cs_pin) & DHGPIO_SUITABLE_PINS) == 0))
		return 0;
	mSPICSPin = cs_pin;
	return 1;
}

void ICACHE_FLASH_ATTR dhspi_write(const char *buf, unsigned int len,
		int disable_cs) {
	dhspi_reinit();

	CLEAR_PERI_REG_MASK(SPI_USER, SPI_USR_MISO);
	SET_PERI_REG_MASK(SPI_USER, SPI_USR_MOSI);
	while (len--) {
		while (READ_PERI_REG(SPI_BASE) & SPI_USR)
			;
		WRITE_PERI_REG(SPI_W0, *buf++);
		SET_PERI_REG_MASK(SPI_BASE, SPI_USR);
	}

	if(disable_cs)
		dhspi_cs_disable();
}

void ICACHE_FLASH_ATTR dhspi_read(char *buf, unsigned int len) {
	dhspi_reinit();

	CLEAR_PERI_REG_MASK(SPI_USER, SPI_USR_MOSI);
	SET_PERI_REG_MASK(SPI_USER, SPI_USR_MISO);
	while (READ_PERI_REG(SPI_BASE) & SPI_USR)
		;
	while (len--) {
		SET_PERI_REG_MASK(SPI_BASE, SPI_USR);
		while (READ_PERI_REG(SPI_BASE) & SPI_USR)
			;
		*buf++ = READ_PERI_REG(SPI_W0) & 0xff;

	}

	dhspi_cs_disable();
}
