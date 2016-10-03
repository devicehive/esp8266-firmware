# Makefile for DeviceHive ESP8266 firmware
# Author Nikolay Khabarov
ifeq ($(OS),Windows_NT)
	CROSS_COMPILE	?= c:/Espressif/xtensa-lx106-elf/bin/xtensa-lx106-elf-
	SINCLUDE		=
else
	CROSS_COMPILE	?= /opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin/xtensa-lx106-elf-
	SINCLUDE		= /usr/include
endif

TARGET			= devicehive
SDKPATH			= ./../sdk
FIRMWAREDIR		= firmware
OBJDIR			= build
TARGETAR		= $(OBJDIR)/$(TARGET).a
TARGETELF		= $(OBJDIR)/$(TARGET).elf
INCLUDEDIRS		= $(addprefix -I,$(SDKPATH)/include sources $(CURDIR) $(SINCLUDE))
LIBDIR			= $(addprefix -L,$(SDKPATH)/lib)
LIBS			= $(addprefix -l,c gcc phy pp net80211 lwip wpa main json)
SOURCES			= $(wildcard sources/*.c) $(wildcard drivers/*.c) $(wildcard sources/devices/*.c) 
OBJECTS			= $(addprefix $(OBJDIR)/, $(SOURCES:%.c=%.o))
CFLAGS			= -O2 -Wpointer-arith -Wundef -Werror -Wunused -mlongcalls -mtext-section-literals -D__ets__ -DICACHE_FLASH -nostdlib
LDFLAGS			= -u call_user_start -Wl,-static -nostdlib
CC				= $(CROSS_COMPILE)gcc
AR				= $(CROSS_COMPILE)ar
SIZE			= $(CROSS_COMPILE)size

.PHONY: all flash terminal clean MAKEBINS

all: $(TARGET) MAKEBINS

$(OBJDIR)/%.o: %.c
	@echo "CC $<"
	@mkdir -p $(dir $@)
	@$(CC) $(INCLUDEDIRS) $(CFLAGS) -c $< -o $@

$(TARGETAR): $(OBJECTS)
	@echo "AR $@"
	@$(AR) cru $@ $(OBJECTS)

$(TARGETELF): $(TARGETAR)
	@$(CC) $(LIBDIR) -T$(SDKPATH)/ld/eagle.app.v6.ld $(LDFLAGS) -Wl,--start-group $(LIBS) $(TARGETAR) -Wl,--end-group -o $@

$(TARGET): $(TARGETELF)
	@echo "LD $@"
	@$(SIZE) -d $(TARGETELF)
	
MAKEBINS: $(TARGET)
	@mkdir -p $(FIRMWAREDIR)
	@./genbin.sh $(TARGETELF) $(FIRMWAREDIR)
	
flash: all
	@(cd $(FIRMWAREDIR) && ./../../esp-utils/build/esp-flasher)
	
terminal:
	@./../esp-utils/build/esp-terminal

rebuild: clean all

clean:
	@rm -rf $(OBJDIR)

