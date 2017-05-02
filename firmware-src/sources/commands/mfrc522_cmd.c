/**
 * @file
 * @brief MFRC522 command handlers.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "commands/mfrc522_cmd.h"
#include "devices/mfrc522.h"
#include "DH/adc.h"

#include "dhcommand_parser.h"
#include <user_interface.h>
#include <osapi.h>
#include <ets_forward.h>

#if defined(DH_COMMANDS_MFRC522) && defined(DH_DEVICE_MFRC522)

/**
 * @brief Do "devices/mfrc522/read" command.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_mfrc522_read(COMMAND_RESULT *cmd_res, const char *command,
                                                      const char *params, unsigned int params_len)
{
	if (params_len) {
		gpio_command_params info;
		ALLOWED_FIELDS fields = 0;
		const char *err_msg = parse_params_pins_set(params, params_len,
				&info, DH_ADC_SUITABLE_PINS, 0, AF_CS, &fields);
		if (err_msg != 0) {
			dh_command_fail(cmd_res, err_msg);
			return; // FAILED
		}
		if (fields & AF_CS) {
			if (MFRC522_Set_CS(info.CS) != MFRC522_STATUS_OK) {
				dh_command_fail(cmd_res, "Unsuitable pin");
				return; // FAILED
			}
		}
	}

	MFRC522_PCD_Init();
	uint8_t bufferATQA[2];
	uint8_t bufferSize = sizeof(bufferATQA);
	MFRC522_StatusCode result = MFRC522_PICC_RequestA(bufferATQA, &bufferSize);
	if (result == MFRC522_STATUS_OK || result == MFRC522_STATUS_COLLISION) {
		MFRC522_Uid *uid = MFRC522_Get_Uid();
		result = MFRC522_PICC_Select(uid, 0);
		if (result == MFRC522_STATUS_OK) {
			char hexbuf[uid->size * 2 + 1];
			unsigned int i;
			for (i = 0; i < uid->size; i++)
				byteToHex(uid->uidByte[i], &hexbuf[i * 2]);
			hexbuf[sizeof(hexbuf) - 1] = 0;
			cmd_res->callback(cmd_res->data, DHSTATUS_OK, RDT_FORMAT_STRING,
					"{\"uid\":\"0x%s\", \"type\":\"%s\"}", hexbuf,
					MFRC522_PICC_GetTypeName(MFRC522_PICC_GetType(uid->sak)));
			MFRC522_PCD_AntennaOff();
			return;
		}
	}

	MFRC522_PICC_HaltA();
	MFRC522_PCD_AntennaOff();
	dh_command_fail(cmd_res, MFRC522_GetStatusCodeName(result));
}


/**
 * @brief Do "devices/mfrc522/mifare/read" and "devices/mfrc522/mifare/write" commands.
 */
void ICACHE_FLASH_ATTR dh_handle_devices_mfrc522_mifare_read_write(COMMAND_RESULT *cmd_res, const char *command,
                                                                   const char *params, unsigned int params_len)
{
	gpio_command_params info;
	ALLOWED_FIELDS fields = 0;
	const int is_write = (0 != os_strcmp(command, "devices/mfrc522/mifare/read"));
	const char *err_msg = parse_params_pins_set(params, params_len,
			&info, DH_ADC_SUITABLE_PINS, 0,
			AF_CS | AF_ADDRESS | AF_KEY | (is_write ? AF_DATA : 0), &fields);
	if (err_msg != 0) {
		dh_command_fail(cmd_res, err_msg);
		return; // FAILED
	}
	if (fields & AF_CS) {
		if (MFRC522_Set_CS(info.CS) != MFRC522_STATUS_OK) {
			dh_command_fail(cmd_res, "Unsuitable pin");
			return; // FAILED
		}
	}
	if ((fields & AF_ADDRESS) == 0) {
		dh_command_fail(cmd_res, "Block address not specified");
		return; // FAILED
	}
	if ((fields & AF_KEY) == 0) {
		// default key
		os_memset(info.storage.key.key_data, 0xFF, MF_KEY_SIZE);
		info.storage.key.key_len = MF_KEY_SIZE;
	} else if(info.storage.key.key_len != MF_KEY_SIZE) {
		dh_command_fail(cmd_res, "Wrong key length");
		return; // FAILED
	}
	if (is_write) {
		if((fields & AF_DATA) == 0) {
			dh_command_fail(cmd_res, "Data not specified");
			return; // FAILED
		} else if(info.data_len != 16) {
			dh_command_fail(cmd_res, "Data length should be 16 bytes");
			return; // FAILED
		}
	}

	MFRC522_PCD_Init();
	uint8_t bufferATQA[2];
	uint8_t bufferSize = sizeof(bufferATQA);
	MFRC522_StatusCode result = MFRC522_PICC_RequestA(bufferATQA, &bufferSize);
	if (result == MFRC522_STATUS_OK || result == MFRC522_STATUS_COLLISION) {
		MFRC522_Uid *uid = MFRC522_Get_Uid();
		result = MFRC522_PICC_Select(uid, 0);
		MIFARE_Key key;
		os_memcpy(key.keyByte, info.storage.key.key_data, MF_KEY_SIZE);
		if (result == MFRC522_STATUS_OK) {
			result = MFRC522_PCD_Authenticate(PICC_CMD_MF_AUTH_KEY_A, info.address, &key, uid);
			if (result == MFRC522_STATUS_OK) {
				uint8_t len = (sizeof(info.data) > 0xFF) ? 0xFF : sizeof(info.data);
				if (is_write)
					result = MFRC522_MIFARE_Write(info.address, (uint8_t*)info.data, info.data_len);
				else
					result = MFRC522_MIFARE_Read(info.address, (uint8_t*)info.data, &len);
				if (result == MFRC522_STATUS_OK) {
					info.count = len;
					if (is_write)
						dh_command_done(cmd_res, "");
					else
						dh_command_done_buf(cmd_res, info.data, info.count);
					MFRC522_PICC_HaltA();
					MFRC522_PCD_StopCrypto1();
					MFRC522_PCD_AntennaOff();
					return;
				}
			}
		}
	}

	MFRC522_PICC_HaltA();
	MFRC522_PCD_StopCrypto1();
	MFRC522_PCD_AntennaOff();
	dh_command_fail(cmd_res, MFRC522_GetStatusCodeName(result));
}

#endif /* DH_COMMANDS_MFRC522 && DH_DEVICE_MFRC522 */
