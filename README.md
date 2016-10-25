# DeviceHive ESP8266 Firmware
Special firmware for usage ESP8266 as local web server with RESTful API and as DeviceHive clouds client.
This repo consist of few parts of this project which can be used with
other projects. Each project has dedicated readme file.

![](images/demo-device.jpg?raw=true)
Photo above is a real photo of the demo device contructed with esp8266. See
http://youtu.be/hzi4djt-wdg

# [DeviceHiveESP8266.md](DeviceHiveESP8266.md)
This is the main documentation file for this firmware. Document contains
commands specification, describes  all features and firmware usage.
[Click here to open it.](DeviceHiveESP8266.md)

# Demo videos
Zero wireless configuring with Android http://youtu.be/2J98YDpbJKo  
DeviceHive clouds demos:    
BH1750 Ambient light http://youtu.be/AkSFdO0soyo  
DS18B20 + iButton + DHT11 http://youtu.be/IuvxwCPNZCc  
Muscle connected to the cloud http://youtu.be/8L96nBNHE14  
SSD1306 OLED display http://youtu.be/gKgU6RzaxFU  
Multple sensor connection http://youtu.be/hzi4djt-wdg  
Smart socket http://youtu.be/hCdzcCjYYMw  
Wireless Configuring with Mac and Linux http://youtu.be/sFD2TTZNNvo  
BMP180 Atmospheric pressure http://youtu.be/P2rxnVjRPyc  
MPU6050 Accelerometer + Gyroscope http://youtu.be/IRj4tOfLTAM  
MFRC522 RFID Reader http://youtu.be/VMT3v49d57g  
Simple relay http://youtu.be/ZLq2X2n_wR0  

# custom firmwares
There is a possibility to build custom firmware on top of this which sends some
notifications to DeviceHive server periodically. In pauses firmware goes to deep
sleep mode. Reimplement `custom_firmware_request()` method in
`custom_firmware.c` file with reading of some sensors data. There are
implementation for some sensors in `sources/devices` dir. Also, it is possible
to create interaction with any sensor using interface implementation modules
(dhi2c, dhspi, dhonewire etc). See `climate` branch as an example.

# esp-utils
Simple utils for flashing and connecting to ESP8266.

# examples-cloud
Simple web pages with JavaScript sample of sendind recieving command
from device via cloud. Onewire DS18B20 examples use 0x33[SKIP ROM] command
which means that examples work only with one currently connected device. Some
browsers(for example Firefox) block cross domen requests even for local file.
Allow temporary cross domain requests in setting or use another browser to run
examples.

Local RESTful API samples are embedded to firmware. Sources of them can be found
at firmware-src/pages directory.

# firmware-src
Sources of DeviceHive ESP8266 firmware.

# firmware-src/genbin.sh
Small utils which is written on bash and can be used on any OS. This util
creates binary firmware files files from crosstool-NG binary file output.

# firmware-tests
Tests for firmware. Youcan find sample of command in file requests.html. Each
addtest() instance have boolean third parameter. If this parameters is true it
means that command and parameters is written correctly and you can use it as
a sample.

# release
Scripts for generating binary releases.

# sdk
SDK from chip manufactor. Included in this repo to make sure that we are
using the same version of this SDK to avoid any surprises from changing APIs

# License
The MIT License. See LICENSE file. Except sdk directory, it has ESPRSSIF MIT 
License, see sdk/License file for details.

# Authors
Nikolay Khabarov
