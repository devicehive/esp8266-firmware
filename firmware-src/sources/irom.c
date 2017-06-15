/** @file
 * @brief Helper module to work with ROM memory.
 * @copyright 2017 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "irom.h"

/*
 * irom_byte() implementation.
 */
uint8 irom_byte(const void *ptr)
{
	if (!is_irom(ptr))
	{
		// for RAM pointers there is no restrictions
		// to access this memory directly
		return *(const uint8_t*)ptr;
	}

	union {
		uint32_t u;   // unsigned integer
		uint8_t b[4]; // bytes
	} tmp;

	// access to ROM memory should be 4-bytes aligned!
	// so read the whole 4-bytes at aligned address
	tmp.u = *(const uint32_t*)((uint32_t)ptr & ~(IROM_FLASH_ALIGNMENT-1));

	// and then, get the requested byte
	return tmp.b[(uint32_t)ptr & (IROM_FLASH_ALIGNMENT-1)];
}


/*
 * irom_read() implementation.
 */
void ICACHE_FLASH_ATTR irom_read(void *ram_buf,
                                 size_t buf_len,
                                 const void *rom_ptr)
{
	// typed pointers
	const uint8_t *rom = (const uint8_t*)rom_ptr;
	      uint8_t *ram = (      uint8_t*)ram_buf;

	while (buf_len > 0) {
		if (((uint32_t)rom & (IROM_FLASH_ALIGNMENT-1)) || buf_len < sizeof(uint32_t)) {
			// if ROM address is not algned or
			// we have less than 4 bytes to copy,
			// then process byte-to-byte...
			*ram = irom_byte(rom);
			buf_len -= 1;
			ram += 1;
			rom += 1;
		} else {
			// ... otherwise process 4-bytes at once
			*((uint32_t*)ram) = *((const uint32_t *)rom);
			buf_len -= sizeof(uint32_t);
			ram += sizeof(uint32_t);
			rom += sizeof(uint32_t);
		}
	}
}


/*
 * irom_cmp() implementation.
 */
int ICACHE_FLASH_ATTR irom_cmp(const void *ram_buf,
                               size_t buf_len,
                               const void *rom_ptr)
{
	// typed pointers
	const uint8_t *rom = (const uint8_t*)rom_ptr;
	      uint8_t *ram = (      uint8_t*)ram_buf;

	while (buf_len > 0) {
		if (((uint32_t)rom & (IROM_FLASH_ALIGNMENT-1)) || buf_len < sizeof(uint32_t)) {
			// if ROM address is not algned or
			// we have less than 4 bytes to compare,
			// then process byte-to-byte...
			const uint8_t b = irom_byte(rom);
			if (*ram != b)
				return 1;

			buf_len -= 1;
			ram += 1;
			rom += 1;
		} else {
			// ... otherwise process 4-bytes at once
			if (*((const uint32_t*)ram) != *((const uint32_t *)rom))
				return 1;

			buf_len -= sizeof(uint32_t);
			ram += sizeof(uint32_t);
			rom += sizeof(uint32_t);
		}
	}

	return 0; // equal
}
