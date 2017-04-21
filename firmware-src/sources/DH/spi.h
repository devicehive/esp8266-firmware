/**
 * @file
 * @brief SPI bus implementation for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DH_SPI_H_
#define _DH_SPI_H_

#include "user_config.h"

#include <c_types.h>

/**
 * @brief SPI operation mode.
 */
typedef enum {
	DH_SPI_CPOL0CPHA0 = 0, ///< @brief Low clock polarity, front edge.
	DH_SPI_CPOL0CPHA1 = 1, ///< @brief Low clock polarity, rear edge.
	DH_SPI_CPOL1CPHA0 = 2, ///< @brief High clock polarity, front edge.
	DH_SPI_CPOL1CPHA1 = 3, ///< @brief High clock polarity, rear edge.
} DHSpiMode;


/**
 * @brief Virtual pin for CS.
 *
 * It means that module will not set up CS line automatically.
 */
#define DH_SPI_NO_CS (~0U)


/**
 * @brief Set SPI mode.
 * @param[in] mode SPI mode.
 * @return Zero on success.
 */
int dh_spi_set_mode(DHSpiMode mode);


/**
 * @brief Set CS pin
 * @param[in] pin Pin number. Can be DH_SPI_NO_CS.
 * @return Zero on success.
 */
int dh_spi_set_cs_pin(unsigned int pin);


/**
 * @brief Write data to SPI bus.
 *
 * Pins will be initialized automatically.
 *
 * @param[in] buf Buffer to write.
 * @param[in] len Buffer length in bytes.
 * @param[in] disable_cs Non zero value means that CS should be disabled after operation, zero value means not.
 */
void dh_spi_write(const void *buf, size_t len, int disable_cs);


/**
 * @brief Read data from SPI bus.
 *
 * Pins will be initialized automatically.
 *
 * CS will be disabled after operation.
 *
 * @param[out] buf Buffer to read data to.
 * @param[in] len Buffer length in bytes.
 */
void dh_spi_read(void *buf, size_t len);


#ifdef DH_COMMANDS_SPI // SPI command handlers
#include "dhsender_data.h"

/**
 * @brief Handle "spi/master/read" command.
 */
void dh_handle_spi_master_read(COMMAND_RESULT *cmd_res, const char *command,
                               const char *params, unsigned int params_len);


/**
 * @brief Handle "spi/master/write" command.
 */
void dh_handle_spi_master_write(COMMAND_RESULT *cmd_res, const char *command,
                                const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_SPI */
#endif /* _DH_SPI_H_ */
