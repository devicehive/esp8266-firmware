/**
 * @file
 * @brief Software I2C implementation for ESP8266 firmware.
 * @copyright 2015 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#ifndef _DH_I2C_H_
#define _DH_I2C_H_

#include "user_config.h"
#include <c_types.h>

/**
 * @brief I2C functions return status.
 */
typedef enum {
	DH_I2C_OK,                  ///< @brief Success.
	DH_I2C_NOACK,               ///< @brief ACK response was not received on bus.
	DH_I2C_WRONG_PARAMETERS,    ///< @brief Wrong parameters.
	DH_I2C_BUS_BUSY,            ///< @brief Module wasn't able to set desired level on bus during specified timeout.
	DH_I2C_DEVICE_ERROR         ///< @brief Error in device.
} DH_I2C_Status;


/**
 * @brief Get error message based on status.
 * @return NULL if status is OK, error message otherwise.
 */
const char* dh_i2c_error_string(int status);


/**
 * @brief Prepare pins for usage with I2C.
 * @param[in] sda_pin Pin number for SDA line.
 * @param[in] scl_pin Pin number for SCL line.
 * @return Status value, one of DH_I2C_Status value.
 */
int dh_i2c_init(unsigned int sda_pin,
                unsigned int scl_pin);


/**
 * @brief Prepare last successfully prepared pins for usage with I2C.
 */
void dh_i2c_reinit(void);


/**
 * @brief Write data to I2C bus.
 * @param[in] address Device address.
 * @param[in] buf Buffer to write.
 * @param[in] len Buffer length in bytes.
 * @param[in] send_stop Send STOP after writing if this parameter has non zero value, do not send if parameter is zero.
 * @return Status value, one of DH_I2C_Status value.
 */
int dh_i2c_write(unsigned int address,
                 const void *buf, size_t len,
                 int send_stop);


/**
 * @brief Read data from I2C bus.
 * @param[in] address Device address.
 * @param[out] buf Buffer to read data to.
 * @param[in] len Buffer length in bytes.
 * @return Status value, one of DH_I2C_Status value.
 */
int dh_i2c_read(unsigned int address,
                void *buf, size_t len);


#ifdef DH_COMMANDS_I2C // I2C command handlers
#include "dhcommand_parser.h"
#include "dhsender_data.h"

/**
 * @brief Helper function to initialize I2C bus.
 * @return Non-zero if I2C was initialized. Zero otherwise.
 */
int dh_i2c_init_helper(COMMAND_RESULT *cmd_res, ALLOWED_FIELDS fields,
                       const gpio_command_params *params);


/**
 * @brief Handle "i2c/master/read" command.
 */
void dh_handle_i2c_master_read(COMMAND_RESULT *cmd_res, const char *command,
                               const char *params, unsigned int params_len);


/**
 * @brief Handle "i2c/master/write" command.
 */
void dh_handle_i2c_master_write(COMMAND_RESULT *cmd_res, const char *command,
                                const char *params, unsigned int params_len);

#endif /* DH_COMMANDS_I2C */
#endif /* _DH_I2C_H_ */
