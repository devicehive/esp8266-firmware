/**
 * @file
 * @brief Simple communication with MHZ-19 CO2 sensor.
 * @copyright 2016 [DeviceHive](http://devicehive.com)
 * @author Nikolay Khabarov
 */
#include "devices/mhz19.h"
#include "DH/uart.h"
#include "dhdebug.h"
#include "dhutils.h"

#include <osapi.h>

#if defined(DH_DEVICE_MHZ19)

/*
 * mhz19_read() implementation.
 */
const char* ICACHE_FLASH_ATTR mhz19_read(int *co2)
{
	dh_uart_set_mode(DH_UART_MODE_PER_BUF);
	if (!!dh_uart_init(9600, 8, 'N', 1))
		return "failed to init UART";

	const uint8_t request[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
	dh_uart_send_buf(request, sizeof(request));
	delay_ms(20);

	uint8_t *result = 0;
	size_t len = dh_uart_get_buf((void**)&result);
	if (len != 9){
		return len ? "Response length mismatch"
		           : "No response";
	}

	int i;
	uint8_t cs = 0;
	for(i = 1; i < 8; i++) {
		cs += result[i];
	}
	cs = ~cs;
	cs++;
	if (cs != result[8]) {
		return "Checksum mismatch";
	}

	*co2 = unsignedInt16be((const char*)result, 2);
	return NULL; // OK
}

#endif /* DH_DEVICE_MHZ19 */
