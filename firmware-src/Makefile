# Makefile for DeviceHive ESP8266 firmware
# Author Nikolay Khabarov
ifeq ($(OS),Windows_NT)
	CROSS_COMPILE	?= c:/Espressif/xtensa-lx106-elf/bin/xtensa-lx106-elf-
else
	CROSS_COMPILE	?= /opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin/xtensa-lx106-elf-
endif

SDKPATH			= $(CURDIR)/../sdk
FIRMWARE		= firmware/devicehive.bin
OBJDIR			= build
TARGETAR		= $(OBJDIR)/devicehive.a
TARGETELF		= $(OBJDIR)/devicehive.elf
INCLUDEDIRS		= $(addprefix -I,$(SDKPATH)/include $(CURDIR)/sources)
LIBDIR			= $(addprefix -L,$(SDKPATH)/lib)
LIBS			= $(addprefix -l,c gcc phy pp net80211 lwip wpa main json crypto)
SOURCES			= $(wildcard sources/*.c) $(wildcard drivers/*.c) $(wildcard sources/devices/*.c) $(wildcard sources/DH/*.c) $(wildcard sources/commands/*.c)
OBJECTS			= $(addprefix $(OBJDIR)/, $(SOURCES:%.c=%.o))
GITVER			= $(shell git rev-parse HEAD 2> /dev/null || echo \"not a git repo\")
CFLAGS			= -O2 -Wall -mlongcalls -mtext-section-literals -D__ets__ -DICACHE_FLASH -nostdlib -std=gnu89 -DFIRMWARE_GIT_REVISION=\"$(GITVER)\"
LDFLAGS			= -u call_user_start -Wl,-static -nostdlib
CC				= $(CROSS_COMPILE)gcc
AR				= $(CROSS_COMPILE)ar
SIZE			= $(CROSS_COMPILE)size
PAGESH			= pages/pages.h

# make SSL=on enables HTTPS/WSS support
ifneq (,$(filter ${SSL},on yes 1))
LIBS			+= $(addprefix -l,ssl)
CFLAGS			+= -DDH_USE_SSL

# if SSL is enabled the firmware size is too big to fit available space
# so we need to disable all supported devices enabled implicitly.
# some of them still can be enabled by defining corresponding macro (see user_config.h)
CFLAGS			+= -DDH_NO_IMPLICIT_DEVICES
endif


.PHONY: all flash full_flash terminal clean disassemble reboot

all: $(FIRMWARE)

$(PAGESH):
	@./pages/gen_pages.sh

$(OBJDIR)/%.o: %.c $(PAGESH)
	@echo "CC $<"
	@mkdir -p $(dir $@)
	@$(CC) $(INCLUDEDIRS) $(CFLAGS) -c $< -o $@

$(TARGETAR): $(OBJECTS)
	@echo "AR $@"
	@$(AR) cru $@ $(OBJECTS)

$(TARGETELF): $(TARGETAR)
	@echo "LD $@"
	@$(CC) $(LIBDIR) -Tdevicehive.ld $(LDFLAGS) -Wl,--start-group $(LIBS) $(TARGETAR) -Wl,--end-group -o $@
	@$(SIZE) -d $(TARGETELF)
	
$(FIRMWARE): $(TARGETELF)
	@mkdir -p $(dir $(FIRMWARE))
	@./genbin.sh $(TARGETELF) $(FIRMWARE)
	
flash: all
	@(cd $(dir $(FIRMWARE)) && ./../../esp-utils/build/esp-flasher --developer)
	@cp -f $(FIRMWARE) $(FIRMWARE).prev

full_flash: rebuild
	@(cd $(dir $(FIRMWARE)) && ./../../esp-utils/build/esp-flasher)
	@cp -f $(FIRMWARE) $(FIRMWARE).prev

terminal:
	@./../esp-utils/build/esp-terminal

rebuild: clean all

clean:
	@rm -rf $(OBJDIR)
	@rm -f $(PAGESH)
	@rm -f $(FIRMWARE).prev

disassemble:
	@$(CROSS_COMPILE)objdump -d $(TARGETELF) > $(OBJDIR)/disassemble.txt

reboot:
	@(cd $(dir $(FIRMWARE)) && ./../../esp-utils/build/esp-flasher --reboot)