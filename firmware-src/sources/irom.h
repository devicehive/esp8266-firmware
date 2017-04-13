/** @file
 * @brief Helper module to work with ROM memory.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 *
 * ESP8266 stores read only variables in RAM by default. It's
 * possible to force compiler to store data in ROM, but reading
 * from ROM is possible only per 32 bits and should be 4 bytes
 * aligned. That's why direct access to this memory via system
 * function causes chip to reboot. This module gives access to
 * this data in easy way.
 */

#ifndef _IROM_H_
#define _IROM_H_

#include <c_types.h>

/**
 * @brief ROM base address.
 */
#define IROM_FLASH_BASE_ADDRESS 0x40200000


/**
 * @brief ROM alignment.
 */
#define IROM_FLASH_ALIGNMENT sizeof(uint32_t)


/**
 * @brief Compile time attribute to store data in ROM memory.
 *
 * Access to this data will be slower, but amount
 * of free RAM will be bigger.
 *
 * @warning Never read such variables directly, except 32 bits.
 */
#define RO_DATA const ICACHE_RODATA_ATTR STORE_ATTR static


/**
 * @brief Check if pointer is stored in ROM.
 * @param[in] ptr Pointer to check.
 * @return Non-zero if pointer is from ROM area.
 */
static inline bool is_irom(const void *ptr)
{
	return (uint32_t)ptr >= IROM_FLASH_BASE_ADDRESS;
}


/**
 * @brief Read single byte from ROM.
 * @param[in] rom_ptr Pointer to data in ROM.
 * @return One byte from ROM.
 */
uint8_t irom_byte(const void *rom_ptr);


/**
 * @brief Read single char from ROM.
 * @param[in] rom_ptr Pointer to data in ROM.
 * @return One char from ROM.
 */
static inline char irom_char(const void *rom_ptr)
{
	return irom_byte(rom_ptr);
}


/**
 * @brief Read multiple bytes from ROM.
 * @param[in,out] ram_buf Pointer in RAM where data will be stored.
 * @param[in] buf_len Number of bytes to read.
 * @param[in] rom_ptr Pointer to data in ROM.
 */
void irom_read(void *ram_buf,
               size_t buf_len,
               const void *rom_ptr);


/**
 * @brief Compare some data with ROM.
 * @param[in] ram_buf Pointer to data in RAM to compare.
 * @param[in] buf_len Number of bytes to compare.
 * @param[in] rom_ptr Pointer to data in ROM.
 * @return Zero if data is equal, non-zero otherwise.
 */
int irom_cmp(const void *ram_buf,
             size_t buf_len,
             const void *rom_ptr);

#endif /* _IROM_H_ */
