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
#define ESP_SECTOR_SIZE 0x1000
#define AUTODETECT_TIMEOUT 1000
#define FLASHING_TIMEOUT 5000
#define AUTODETECT_MAX_PORT 20
#define AUTODETECT_MAX_SYNC_ATTEMPS 3
#define MAX_SYNC_ATTEMPS 3
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

uint8_t ESP_INIT_DATA_DEAFULT[] = {	0x05, 0x00, 0x04, 0x02, 0x05, 0x05, 0x05, 0x02, 0x05, 0x00, 0x04, 0x05, 0x05, 0x04, 0x05, 0x05,
									0x04, 0xfe, 0xfd, 0xff, 0xf0, 0xf0, 0xf0, 0xe0, 0xe0, 0xe0, 0xe1, 0x0a, 0xff, 0xff, 0xf8, 0x00,
									0xf8, 0xf8, 0x52, 0x4e, 0x4a, 0x44, 0x40, 0x38, 0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x04, 0x05,
									0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
									0xe1, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x93, 0x43, 0x00, 0x00, 0x00,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
									0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


void SerialPortError(SerialPort */*port*/,const char *text) {
	printf("\r\n%s\r\n", text);
}

uint8_t recivedBuf[4096];
unsigned int recivedPos = 0;
bool recivedEscape = false;
SerialPort *mCurrentPort = NULL;

void SerialPortRecieved(SerialPort *port, const char *text, unsigned int len) {
	if(mCurrentPort && mCurrentPort != port)
		return;
	for (unsigned int i = 0; i < len; i++) {
		uint8_t c = text[i];
		//printf("0x%02X \r\n", (uint32_t)c);
		if(recivedPos >= sizeof(recivedBuf))
			break;
		if(recivedEscape) {
			if(c == 0xdc) {
				recivedBuf[recivedPos++] = 0xc0;
			} else if(c == 0xdd) {
				recivedBuf[recivedPos++] = 0xdb;
			}
			recivedEscape = false;
		} else if(c == 0xdb) {
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
		if(i >= sizeof(ESP_REQUEST_HEADER)) {
			c = ((uint8_t *) body)[i - sizeof(ESP_REQUEST_HEADER)];
		} else {
			c = hdrp[i];
		}
		if(c == 0xdb) {
			oBuf[oBufPos++] = 0xdb;
			oBuf[oBufPos++] = 0xdd;
		} else if(c == 0xc0) {
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
	if(!port->waitAnswer(5, TIMEOUT)) {
		if(printErrors)
			printf("\r\nNo answer from device\r\n");
		return false;
	}
	if(recivedBuf[0] != 0xc0 && recivedBuf[1] != 0x01
			&& recivedBuf[1] != hdr->command) {
		if(printErrors)
			printf("\r\nWrong answer\r\n");
		return false;
	}
	uint32_t asize = (uint32_t) recivedBuf[3] | (uint32_t) (recivedBuf[4] << 8);
	if(!port->waitAnswer(asize + 10, TIMEOUT)) {
		if(printErrors)
			printf("\r\nAnswer not completed\r\n");
		return false;
	}
	port->waitTransmitionEnd(100);
	if(asize != 2 || recivedBuf[asize + 9] != 0xc0) {
		if(printErrors)
			printf("\r\nWrong body length\r\n");
		return false;
	}
	if(recivedBuf[asize + 7] != 0x0) {
		if(printErrors)
			printf("\r\nOperation 0x%02X failed, code: 0x%02X\r\n", recivedBuf[2],
				recivedBuf[asize + 8]);
		return false;
	}
	return true;
}

bool flash_start(SerialPort *port, uint32_t blocks_count, uint32_t size, uint32_t address) {
	ESP_REQUEST_HEADER rh;
	uint32_t pbody[4];

	// workaround for SPIEraseArea
	const uint32_t sectors_count = ceil((double) size / (double) ESP_SECTOR_SIZE);
	uint32_t fix = 16  - address / ESP_SECTOR_SIZE % 16;
	if(sectors_count < fix)
		fix = sectors_count;

	pbody[0] = htole32(((sectors_count < 2 * fix) ? (sectors_count + 1) / 2 : (sectors_count - fix)) * ESP_SECTOR_SIZE);
	pbody[1] = htole32(blocks_count);
	pbody[2] = htole32(ESP_BLOCK_SIZE);
	pbody[3] = htole32(address);

	rh.magic = 0x00;
	rh.command = 0x02; // prepare flash
	rh.size = htole16(sizeof(pbody));
	rh.cs = esp_checksum(pbody, sizeof(pbody));

	if(!flash_send(port, &rh, &pbody, true))
		return false;

	return true;
}

/* Data should be aligned  per 4096 bytes */
bool flash_mem(SerialPort *port, uint8_t * buf, uint32_t size, uint32_t address) {
	if(size % 4096 || address % 4096) {
		printf("\r\nINTERNAL ERROR: data isn't align by 4096 bytes - %d\r\n", size);
		return false;
	}
	uint32_t blocks_count = ceil((double) size / (double) ESP_BLOCK_SIZE);
	ESP_REQUEST_HEADER rh;

	if(!flash_start(port, blocks_count, size, address)) {
		printf("\r\nFailed to enter flash mode\r\n");
		return false;
	}

	for (unsigned int seq = 0; seq < blocks_count; seq++) {
		printf("\rWriting block %d/%d at 0x%08X        ", seq, blocks_count, address + ESP_BLOCK_SIZE * seq);
		uint8_t buffer[ESP_BLOCK_SIZE + 16];
		uint8_t *data = (uint8_t *) &buffer[16];
		uint32_t *data32 = (uint32_t *) buffer;
		data32[0] = htole32(ESP_BLOCK_SIZE);
		data32[1] = htole32(seq);
		data32[2] = 0;
		data32[3] = 0;
		uint32_t bl = size - seq * ESP_BLOCK_SIZE;
		if(bl < ESP_BLOCK_SIZE) {
			memcpy(data, &buf[seq * ESP_BLOCK_SIZE], bl);
			memset(&data[bl], 0xff, ESP_BLOCK_SIZE - bl);
		} else {
			memcpy(data, &buf[seq * ESP_BLOCK_SIZE], ESP_BLOCK_SIZE);
		}
		rh.magic = 0x00;
		rh.command = 0x03; // flash data
		rh.size = htole16(sizeof(buffer));
		rh.cs = esp_checksum(data, ESP_BLOCK_SIZE);
		if(!flash_send(port, &rh, buffer, true)) {
			printf("\r\nFailed to flash\r\n");
			return false;
		}
	}
	printf("\rBlocks wrote %d/%d at 0x%08X\r\n", blocks_count, blocks_count, address);
	return true;
}

bool flash_file(SerialPort *port, char *file, uint32_t address, char *incremental) {
	const uint32_t FLASH_ALIGN = 4096;
	FILE* fd = fopen(file, "rb");
	if(!fd) {
		printf("\r\nFailed to open file %s\r\n", file);
		return false;
	}
	fseek(fd, 0, SEEK_END);
	uint32_t size = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	printf("Flashing %s at 0x%08X\r\n", file, address);
	uint32_t extra = FLASH_ALIGN * (uint32_t)ceil((double) size / (double) FLASH_ALIGN) - size;
	printf("Align with extra %d bytes, total %d bytes\r\n", extra, extra + size);
	uint8_t *data = new uint8_t[size + extra];
	memset(&data[size], 0xFF, extra);
	uint8_t *incremental_data = NULL;

	unsigned int rb = fread(data, 1, size, fd);
	fclose (fd);
	if(rb != size) {
		delete [] data;
		printf("\r\nFailed to read image file\r\n");
		return false;
	}
	if(incremental) {
		FILE* fdi = fopen(incremental, "rb");
		if(fdi) {
			incremental_data = new uint8_t[size + extra];
			memset(&incremental_data[size], 0xFF, extra);
			// read no more then data size
			unsigned int rbi = fread(incremental_data, 1, size, fdi);
			if(rbi) {
				while(rbi < size) { // fill tail with anything but not data
					incremental_data[rbi] = ~data[rbi];
					rbi++;
				}
			} else {
				delete [] incremental_data;
				incremental_data = NULL;
			}
			fclose(fdi);
		}
		if(incremental_data == NULL) {
			printf("Failed to open previous file to do incremental flash. Performing full flash.\r\n");
		} else {
			printf("Previously written data found, skipping the same sectors.\r\n");
			if(memcmp(data, incremental_data, size) == 0) {
				printf("Incremental data is the same.\r\n");
				return true;
			}
		}
	}
	size += extra;
	bool res = false;
	if(incremental_data) {
		for(int i = 0; i < 2; i++) {
			uint32_t total = 0;
			uint32_t blocks = 0;
			const uint32_t ERASE_AREA_ALIG = FLASH_ALIGN * 4;
			for(uint32_t offset = 0; offset < size; offset += FLASH_ALIGN) {
				uint32_t towrite = 0;
				while(offset + towrite < size) {
					if(memcmp(&data[offset + towrite], &incremental_data[offset + towrite], FLASH_ALIGN) == 0)
						break;
					towrite += FLASH_ALIGN;
				}
				if(towrite) {
					// esp erases some more data then specified due SPIEraseArea issue, 16 kb blocks work normally
					if(towrite % ERASE_AREA_ALIG) {
						towrite = (towrite / ERASE_AREA_ALIG + 1) * ERASE_AREA_ALIG;
					}
					// allow to write less at the end
					if(towrite + offset > size)
						towrite = size - offset;

					blocks++;
					if(i != 0) { // first run is dry
						res = flash_mem(port, &data[offset], towrite, address + offset);
						if(!res)
							return res;
					}
					offset += towrite;
					total += towrite;
				}
			}
			if(i == 0 && blocks > 3) {
				printf("Too many difference, performing full flash\r\n");
				res = flash_mem(port, data, size, address);
				break;
			} else if(blocks > 3) {
				printf("Incremental flash wrote %d/%d bytes in %d blocks.\r\n", total, size, blocks);
			}
		}
	} else {
		res = flash_mem(port, data, size, address);
	}

	delete [] data;
	if(incremental_data)
		delete [] incremental_data;
	return res;
}

bool flash_done(SerialPort *port) {
	uint32_t reboot = htole32(1);
	ESP_REQUEST_HEADER rh;
	rh.magic = 0x00;
	rh.command = 0x04; // flash done
	rh.size = htole16(sizeof(reboot));
	rh.cs = esp_checksum(&reboot, sizeof(reboot));
	if(!flash_send(port, &rh, &reboot, true)) {
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
	for(int i=0; i < MAX_SYNC_ATTEMPS; i++) {
		if(flash_send(port, &rh, (void*)body, false)) {
			return true;
		}
	}
	if(printErrors)
		printf("\r\nFailed to sync device\r\n");
	return false;
}

bool startWith(const char *str, const char *start) {
	return strncmp(str, start, strlen(start)) == 0;
}

int exit() {
	printf("Press ENTER for exit.\r\n");
	getchar();
	return 1;
}

bool flash_default_config(SerialPort *port) {
	printf("Flashing default configuration at 0x%08X\r\n", 0x7C000);
	uint8_t data[4*1024];
	memcpy(data, ESP_INIT_DATA_DEAFULT, sizeof(ESP_INIT_DATA_DEAFULT));
	memset(&data[sizeof(ESP_INIT_DATA_DEAFULT)], 0xFF, sizeof(data) - sizeof(ESP_INIT_DATA_DEAFULT));
	return flash_mem(port, data, sizeof(data), 0x7C000);
}

void force_flash_mode(SerialPort *port) {
	// Typically dev boards have:
	// RTS is connected to GPIO0
	// DTR is connected to RTS
    port->setDtr(false);
    port->setRts(true);
    port->sleep(50);
    port->setDtr(true);
    port->setRts(false);
    port->sleep(50);
    port->setDtr(false);
}

void reboot(SerialPort *port) {
	// Typically dev boards have:
	// RTS is connected to GPIO0
	// DTR is connected to RTS
    port->setDtr(false);
    port->sleep(50);
    port->setDtr(true);
    port->sleep(50);
    port->setDtr(false);
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
		force_flash_mode(port);
		if(!flash_sync(port, true)) {
			delete port;
			printf( "Device is not connected to UART or not in boot mode.\r\n" \
					CHECK_BOOT_MODE_NOTIFY"\r\n");
			return exit();
		}
	} else {
		TIMEOUT = AUTODETECT_TIMEOUT;
		printf("Detecting device...\r\n");
		SerialPort *ports[AUTODETECT_MAX_PORT];
		for (uint32_t i = 0; i < AUTODETECT_MAX_PORT; i++) {
			ports[i] = NULL;
		}
		port = NULL;
		for(uint32_t j = 0; port == NULL && j < AUTODETECT_MAX_SYNC_ATTEMPS; j++) {
			for(uint32_t i = 0; port == NULL && i < AUTODETECT_MAX_PORT; i++) {
				if (!ports[i]) {
					const char *name = SerialPort::findNextPort(false);
					if(name[0] == 0)
						break;
					ports[i] = SerialPort::open(name);
				}
				if (ports[i]) {
					mCurrentPort = ports[i];
					if(j == 0 && port == NULL)
						force_flash_mode(ports[i]);
					if (flash_sync(ports[i], false)) {
						printf("Device found on %s and successfully synced\r\n", ports[i]->getName());
						port = ports[i];
					}
					mCurrentPort = NULL;
				}
			}
		}
		SerialPort::findNextPort(true);
		for (uint32_t i = 0; i < AUTODETECT_MAX_PORT; i++) {
			if (ports[i] && ports[i] != port) {
				delete ports[i];
			}
		}
		TIMEOUT = FLASHING_TIMEOUT;
		if(!port) {
			printf( "Can not detect port. Check if device connected and driver is installed.\r\n" \
					"If port number greater than %d, specify port manualy as arg.\r\n" \
					CHECK_BOOT_MODE_NOTIFY"\r\n", AUTODETECT_MAX_PORT);
			return exit();
		}
	}
	flash_sync(port, false);

	bool developerMode = false;
	if(argc > currentArg) {
		if(strncmp(argv[currentArg], "--developer", 5) == 0) {
			currentArg++;
			developerMode = true;
		} else if(strncmp(argv[currentArg], "--reboot", 5) == 0) {
			reboot(port);
			return 0;
		}
	}

	if(currentArg >= argc) {
		if(developerMode) {
				printf( "Developer mode.\r\n"
						"Flashing:\r\n" \
						"0x00000 <- devicehive.bin, using devicehive.bin.prev as previously written data\r\n" \
						"If chip was changed, please perform full flash first.\r\n"
						);
		} else {
			printf( "No image file were specified. You can specify it in args by pairs\r\n" \
					"hex adress and image file name, for example:\r\n" \
					"esp-flasher 0x00000 image1.bin 0x40000 image2.bin\r\n"
					"Trying to flash with defaults:\r\n" \
					"0x00000 <- devicehive.bin\r\n" \
					"0x7C000 <- default configuration\r\n"
					);
		}
		char dh[] = "devicehive.bin";
		int l = strlen(argv[0]);
		char df[l + sizeof(dh)];
		char *filename = dh;
		FILE *fl = fopen(dh, "rb");
		if(fl) { // if in current dir present
			fclose(fl);
		} else { // check the directory with binary
			strcpy(df, argv[0]);
			while(l >= 0) {
				if(df[l] == '/' || df[l] == '\\') {
					strcpy(&df[l + 1], dh);
					filename = df;
					break;
				}
				l--;
			}
		}
		isSuccess = flash_file(port, filename, 0x00000,
				developerMode ? (char*)"devicehive.bin.prev" : NULL);
		if(isSuccess && !developerMode)
			isSuccess = flash_default_config(port);
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
				isSuccess = flash_file(port, argv[currentArg + 1], address, NULL);
			}
		}
	}

	if(isSuccess) {
		flash_start(port, 0, 0, 0);
		isSuccess = flash_done(port);
	}

	delete port;
	if(isSuccess)
		printf("Flashing done.\r\n");
	else
		printf("Flashing failed.\r\n");
	exit();
	return isSuccess? 0 : 1;
}
