/*
 * main.cpp
 *
 * Copyright 2015 DeviceHive
 *
 * Author: Nikolay Khabarov
 *
 * Description: Main file of esp-flasher
 *
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <stdint.h>

#include "common/serialport.h"

#define ESP_BLOCK_SIZE 0x400
#define AUTODETECT_TIMEOUT 1000
#define FLASHING_TIMEOUT 5000
#define AUTODETECT_MAX_PORT 20
#define AUTODETECT_MAX_SYNC_ATTEMPS 3
#define CHECK_BOOT_MODE_NOTIFY	"Make sure that GPIO0 and GPIO15 are connected to low-level(ground),\r\n" \
								"GPIO2 and CH_PD are connected to high-level(power source)\r\n" \
								"And reboot device(reconnect power or connect RST pin to ground for a second).\r\n"

unsigned int TIMEOUT = FLASHING_TIMEOUT;

typedef struct {
	uint8_t magic;
	uint8_t command;
	uint16_t size;
	uint32_t cs;
} ESP_REQUEST_HEADER;


void SerialPortError(SerialPort */*port*/,const char *text) {
	printf("\r\n%s\r\n", text);
}

uint8_t recivedBuf[4096];
unsigned int recivedPos = 0;
bool recivedEscape = false;
void SerialPortRecieved(SerialPort *port, const char *text, unsigned int len) {
	for (unsigned int i = 0; i < len; i++) {
		uint8_t c = text[i];
		//printf("0x%02X \r\n", (uint32_t)c);
		if (recivedPos >= sizeof(recivedBuf))
			break;
		if (recivedEscape) {
			if (c == 0xdc) {
				recivedBuf[recivedPos++] = 0xc0;
			} else if (c == 0xdd) {
				recivedBuf[recivedPos++] = 0xdb;
			} else {
				SerialPortError(port, (char*) "Invalid escape uint8_t");
			}
			recivedEscape = false;
		} else if (c == 0xdb) {
			recivedEscape = true;
		} else {
			recivedBuf[recivedPos++] = c;
		}
	}
}

void clearReciveBuffer() {
	recivedPos = 0;
	recivedEscape = false;
}

uint16_t htole16(uint16_t n) {
	uint32_t res;
	uint8_t *p = (uint8_t *) &res;
	p[0] = n % 0x100;
	n /= 0x100;
	p[1] = n % 0x100;
	return res;
}

uint32_t htole32(uint32_t n) {
	uint32_t res;
	uint8_t *p = (uint8_t *) &res;
	for (unsigned int i = 0; i < sizeof(uint32_t); i++) {
		p[i] = n % 0x100;
		n /= 0x100;
	}
	return res;
}

uint32_t esp_checksum(void *data, unsigned int len) {
	uint8_t res = 0xef;
	for (unsigned int i = 0; i < len; i++) {
		res ^= ((uint8_t*) data)[i];
	}
	return htole32(res);
}

bool flash_send(SerialPort *port, ESP_REQUEST_HEADER *hdr, void *body, bool printErrors) {
	clearReciveBuffer();
	uint8_t *hdrp = (uint8_t *) hdr;
	uint8_t oBuf[(sizeof(ESP_REQUEST_HEADER) + hdr->size) * 2];
	unsigned int oBufPos = 0;
	oBuf[oBufPos++] = 0xc0;
	for (unsigned int i = 0; i < sizeof(ESP_REQUEST_HEADER) + hdr->size; i++) {
		uint8_t c;
		if (i >= sizeof(ESP_REQUEST_HEADER)) {
			c = ((uint8_t *) body)[i - sizeof(ESP_REQUEST_HEADER)];
		} else {
			c = hdrp[i];
		}
		if (c == 0xdb) {
			oBuf[oBufPos++] = 0xdb;
			oBuf[oBufPos++] = 0xdd;
		} else if (c == 0xc0) {
			oBuf[oBufPos++] = 0xdb;
			oBuf[oBufPos++] = 0xdc;
		} else {
			oBuf[oBufPos++] = c;
		}
	}
	oBuf[oBufPos++] = 0xc0;

//	for(uint32_t i=0; i < oBufPos; i++) {
//		printf("%02X ", oBuf[i]);
//	}
//	printf("\r\n");

	port->send(oBuf, oBufPos);
	if (!port->waitAnswer(9, TIMEOUT)) {
		if(printErrors)
			printf("\r\nNo answer from device\r\n");
		return false;
	}
	if (recivedBuf[0] != 0xc0 && recivedBuf[1] != 0x01
			&& recivedBuf[1] != hdr->command) {
		if(printErrors)
			printf("\r\nWrong answer\r\n");
		return false;
	}
	uint32_t asize = (uint32_t) recivedBuf[3] | (uint32_t) (recivedBuf[4] << 8);
	if (!port->waitAnswer(asize + 3, TIMEOUT)) {
		if(printErrors)
			printf("\r\nAnswer not completed\r\n");
		return false;
	}
	if (asize != 2 || recivedBuf[asize + 9] != 0xc0) {
		if(printErrors)
			printf("\r\nWrong body length\r\n");
		return false;
	}
	if (recivedBuf[asize + 7] != 0x0) {
		if(printErrors)
			printf("\r\nOperation 0x%02X failed, code: 0x%02X\r\n", recivedBuf[2],
				recivedBuf[asize + 8]);
		return false;
	}
	return true;
}

bool flash_file(SerialPort *port, char *file, uint32_t address) {
	FILE* fd = fopen(file, "rb");
	if (!fd) {
		printf("\r\nFailed to open file %s\r\n", file);
		return false;
	}
	fseek(fd, 0, SEEK_END);
	uint32_t size = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	printf("Flashing %s at 0x%08X\r\n", file, address);
	uint32_t blocks_count = ceil((double) size / (double) ESP_BLOCK_SIZE);
	ESP_REQUEST_HEADER rh;
	uint32_t pbody[4];
	pbody[0] = htole32(ESP_BLOCK_SIZE * blocks_count);
	pbody[1] = htole32(blocks_count);
	pbody[2] = htole32(ESP_BLOCK_SIZE);
	pbody[3] = htole32(address);

	rh.magic = 0x00;
	rh.command = 0x02; // prepare flash
	rh.size = htole16(sizeof(pbody));
	rh.cs = esp_checksum(pbody, sizeof(pbody));

	if (!flash_send(port, &rh, &pbody, true)) {
		printf("\r\nFailed to enter flash mode\r\n");
		return false;
	}
	printf("Total block count %d, block size %d\r\n", blocks_count,
			ESP_BLOCK_SIZE);

	for (unsigned int seq = 0; seq < blocks_count; seq++) {
		printf("\rWriting block %d/%d at 0x%08X        ", seq, blocks_count,
				address + ESP_BLOCK_SIZE * seq);
		uint8_t buffer[ESP_BLOCK_SIZE + 16];
		uint8_t *data = (uint8_t *) &buffer[16];
		uint32_t *data32 = (uint32_t *) buffer;
		data32[0] = htole32(ESP_BLOCK_SIZE);
		data32[1] = htole32(seq);
		data32[2] = 0;
		data32[3] = 0;
		unsigned int rb = fread(data, 1, ESP_BLOCK_SIZE, fd);
		if(rb == 0) {
			fclose(fd);
			printf("\r\nFailed to read image file\r\n");
			return false;
		} else if (rb < ESP_BLOCK_SIZE) {
			memset(&data[rb], 0xff, ESP_BLOCK_SIZE - rb);
		}
		rh.magic = 0x00;
		rh.command = 0x03; // flash data
		rh.size = htole16(sizeof(buffer));
		rh.cs = esp_checksum(data, ESP_BLOCK_SIZE);
		if (!flash_send(port, &rh, buffer, true)) {
			fclose(fd);
			printf("\r\nFailed to flash\r\n");
			return false;
		}
	}
	printf("\rBlocks wrote %d/%d at 0x%08X\r\n", blocks_count, blocks_count,
			address);

	fclose(fd);
	return true;
}

bool flash_done(SerialPort *port) {
	uint32_t reboot = htole32(1);
	ESP_REQUEST_HEADER rh;
	rh.magic = 0x00;
	rh.command = 0x04; // flash done
	rh.size = htole16(sizeof(reboot));
	rh.cs = esp_checksum(&reboot, sizeof(reboot));
	if (!flash_send(port, &rh, &reboot, true)) {
		printf("\r\nFailed to finish flash\r\n");
		return false;
	}
	return true;
}

bool flash_sync(SerialPort *port, bool printErrors) {
	const uint8_t body [] = {0x07, 0x07, 0x12, 0x20,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
			0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
	ESP_REQUEST_HEADER rh;
	rh.magic = 0x00;
	rh.command = 0x08; // sync
	rh.size = htole16(sizeof(body));
	rh.cs = esp_checksum((void*)body, sizeof(body));
	if (!flash_send(port, &rh, (void*)body, printErrors)) {
		if(printErrors)
			printf("\r\nFailed to sync device\r\n");
		return false;
	}
	return true;
}

bool startWith(const char *str, const char *start) {
	return strncmp(str, start, strlen(start)) == 0;
}

int exit() {
	printf("Press ENTER for exit.\r\n");
	getchar();
	return 1;
}

int main(int argc, char* argv[]) {
	setbuf(stdout, NULL);
	int currentArg = 1;
	bool isSuccess = true;
	SerialPort *port = NULL;
	if(argc > 1) {
		if(
			#ifdef COMWINDOWS
			strnicmp(argv[1], "COM", 3) == 0
			#else
			strncmp(argv[1], "/dev/", 5) == 0
			#endif
												) {
			for(uint32_t j = 0; port == NULL && j < AUTODETECT_MAX_SYNC_ATTEMPS; j++) {
				port = SerialPort::open(argv[1]);
			}
			if(!port) {
				printf("Can not open port %s.\r\n", argv[1]);
				return exit();
			}
			currentArg++;
		}
	}
	if(port) {
		if(!flash_sync(port, true)) {
			delete port;
			printf( "Device is not connected to UART or not in boot mode.\r\n" \
					CHECK_BOOT_MODE_NOTIFY"\r\n");
			return exit();
		}
	} else {
		TIMEOUT = AUTODETECT_TIMEOUT;
		printf("Detecting device...\r\n");
		for(uint32_t j = 0; j < AUTODETECT_MAX_SYNC_ATTEMPS; j++) {
			for(uint32_t i = 0; i < AUTODETECT_MAX_PORT; i++) {
				const char *name = SerialPort::findNextPort(false);
				if(name[0] == 0)
					break;
				port = SerialPort::open(name);
				if(port) {
					if(flash_sync(port, false)) {
						printf("Device found on %s and successfully synced\r\n", name);
						SerialPort::findNextPort(true);
						goto portfound;
					}
					delete port;
					port = NULL;
				}
			}
			SerialPort::findNextPort(true);
		}
		portfound:
		TIMEOUT = FLASHING_TIMEOUT;
		if(!port) {
			printf( "Can not detect port. Check if device connected and driver is installed.\r\n" \
					"If port number greater than %d, specify port manualy as arg.\r\n" \
					CHECK_BOOT_MODE_NOTIFY"\r\n", AUTODETECT_MAX_PORT);
			return exit();
		}
	}
	flash_sync(port, false);

	if(currentArg >= argc) {
		printf( "No image file were specified. You can specify it in args by pairs\r\n" \
				"hex adress and image file name, for example:\r\n" \
				"esp-flasher 0x00000 image1.bin 0x40000 image2.bin\r\n"
				"Trying to flash with defaults:\r\n" \
				"0x00000 <- 0x00000.bin\r\n" \
				"0x40000 <- 0x40000.bin\r\n");
				isSuccess = flash_file(port, (char*)"0x00000.bin", 0x00000);
				if(isSuccess)
					isSuccess = flash_file(port, (char*)"0x40000.bin", 0x40000);
	} else {
		if((argc - currentArg) % 2) {
			delete port;
			printf("Wrong arguments. Each address have to have corresponding image file in args.\r\n");
			return exit();
		}
		printf("Will make an attempt to flash at:\r\n");
		for(int tmp =  currentArg; tmp < argc - 1; tmp += 2) {
			uint32_t address;
			if(sscanf(argv[tmp], "%x", &address)) {
				printf("0x%05X <- %s\r\n", address, argv[tmp + 1]);
				FILE *fl = fopen(argv[tmp + 1], "rb");
				if(!fl) {
					delete port;
					printf("%s file not found.\r\n", argv[tmp + 1]);
					return exit();
				}
				fclose(fl);
			} else {
				delete port;
				printf("%s <- %s\r\n", argv[tmp], argv[tmp + 1]);
				printf("%s is not hex address.\r\n", argv[tmp]);
				return exit();
			}
		}
		printf("-----------------------\r\n");
		for( ; currentArg < argc - 1 && isSuccess; currentArg += 2) {
			uint32_t address;
			if(sscanf(argv[currentArg], "%x", &address)) {
				isSuccess = flash_file(port, argv[currentArg + 1], address);
			}
		}
	}

	if(isSuccess) {
		isSuccess = flash_done(port);
	}

	delete port;
	if(isSuccess)
		printf("Flashing done.\r\n");
	else
		printf("Flashing failed.\r\n");
	exit();
	return 0;
}
