# Makefile for DeviceHive ESP8266 firmware
# Author Nikolay Khabarov
ifeq ($(OS),Windows_NT)
	CROSS_COMPILE	?= c:/Espressif/xtensa-lx106-elf/bin/xtensa-lx106-elf-
	SINCLUDE		=
else
	CROSS_COMPILE	?= /opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin/xtensa-lx106-elf-
	SINCLUDE		= /usr/include
endif

SDKPATH			= ./../sdk
FIRMWARE		= firmware/devicehive.bin
OBJDIR			= build
TARGETAR		= $(OBJDIR)/devicehive.a
TARGETELF		= $(OBJDIR)/devicehive.elf
INCLUDEDIRS		= $(addprefix -I,$(SDKPATH)/include sources $(CURDIR) $(SINCLUDE))
LIBDIR			= $(addprefix -L,$(SDKPATH)/lib)
LIBS			= $(addprefix -l,c gcc phy pp net80211 lwip wpa main json crypto)
SOURCES			= $(wildcard sources/*.c) $(wildcard drivers/*.c) $(wildcard sources/devices/*.c) 
OBJECTS			= $(addprefix $(OBJDIR)/, $(SOURCES:%.c=%.o))
CFLAGS			= -O2 -Wpointer-arith -Wundef -Werror -Wunused -mlongcalls -mtext-section-literals -D__ets__ -DICACHE_FLASH -nostdlib
LDFLAGS			= -u call_user_start -Wl,-static -nostdlib
CC				= $(CROSS_COMPILE)gcc
AR				= $(CROSS_COMPILE)ar
SIZE			= $(CROSS_COMPILE)size

.PHONY: all flash terminal clean

all: $(FIRMWARE)

$(OBJDIR)/%.o: %.c
	@echo "CC $<"
	@mkdir -p $(dir $@)
	@$(CC) $(INCLUDEDIRS) $(CFLAGS) -c $< -o $@

$(TARGETAR): $(OBJECTS)
	@echo "AR $@"
	@$(AR) cru $@ $(OBJECTS)

$(TARGETELF): $(TARGETAR)
	@echo "LD $@"
	@$(CC) $(LIBDIR) -T$(SDKPATH)/ld/eagle.app.v6.ld $(LDFLAGS) -Wl,--start-group $(LIBS) $(TARGETAR) -Wl,--end-group -o $@
	@$(SIZE) -d $(TARGETELF)
	
$(FIRMWARE): $(TARGETELF)
	@mkdir -p $(dir $(FIRMWARE))
	@./genbin.sh $(TARGETELF) $(FIRMWARE)
	@echo -n "Firmware file is $(FIRMWARE), size is "
	@stat -c '%s bytes' $(FIRMWARE)
	
flash: all
	@(cd $(dir $(FIRMWARE)) && ./../../esp-utils/build/esp-flasher)
	
terminal:
	@./../esp-utils/build/esp-terminal

rebuild: clean all

clean:
	@rm -rf $(OBJDIR)

