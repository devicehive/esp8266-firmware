# DeviceHive ESP8266 Firmware
Special firmware to use ESP8266 in DeviceHive clouds.

# Building
Project can be opened in Eclipse. To compile it you would need to install
toolchain. See instructions [here](https://github.com/esp8266/esp8266-wiki/wiki/Toolchain).
Actually we need just: [Xtensa crosstool-NG](https://github.com/jcmvbkbc/crosstool-NG).
SDK is already included in this repo.

# Firmware usage
Flash firmware (`devicehive.bin` firmware directory) to the device with
`esp-flasher` util (see `esp-util` project on top of the repo). You can also use any other
flasher for `esp8266` ([esptool](https://github.com/themadinventor/esptool) for example).

Firmware should be flashed at `0x0` SPI chip address. Connect to device terminal
via serial port with `esp-terminal` (see `esp-util` project). You can also use any 
other serial port terminal with escape sequences support. [PuTTY](http://www.putty.org/)
for Windows OS or `screen` util under Linux for example. This
firmware has quite regular unix like terminal via UART. Autocomplete and command
history are supported.
Type `help` in terminal to see all aviable commands. At first you need to configure
device. Type `configure` in terminal to run configuration util on it. Enter network
and server parameters, device will be rebooted. Now you can send command via
DeviceHive cloud to device. Please refer to [ESP8266](../DeviceHiveESP8266.md) firmware
commands documentation to find which commands are supported.

# genbin.sh
Small util to extract binary images for flashing for compiled file. Might be useful
for other projects. Usage:

```
genbin.sh <path to elf file> [<output directory>]
```

# pages
This directory contains web pages which will be embedded into firmware. Rebuild
(`make rebuild`) sources after changing content of this pages or run mannually
`gen_pages.sh` script to update it.

# License
MIT. See [LICENSE](./LICENSE) file.
