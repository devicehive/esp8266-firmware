# DeviceHive ESP8266 Firmware
Special firmware for usage ESP8266 in DeviceHive clouds.
This repo consist of few parts of this project which can be used with
other projects. Each project has dedicated readme file.

# esp-utils
Simple utils for flashing and connecting to ESP8266.

# examples
Simple web pages with JavaScript sample of sendind recieving command
from device via cloud.

# firmware-src
Sources of DeviceHive ESP8266 firmware.

# firmware-src/genbin.sh
Small utils which is written on bash and can be used on any OS. This util
creates binary firmware files files from crosstool-NG binary file output.

# release
Pre built binaries of utils and released firmwares.

# sdk
SDK from chip manufactor. Included in this repo to make sure that we are
using the same version of this SDK to avoid any surprises from changing APIs

# DeviceHiveESP8266commands.pdf
Document contains cloud commands specification for this firmware.

# License
The MIT License. See LICENSE file. Except sdk directory, it has ESPRSSIF MIT 
License, see sdk/License file for details.

# Authors
Nikolay Khabarov
