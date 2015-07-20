# DeviceHive ESP8266 Simple utils
esp-terminal:
Simple tool for easy access to DeviceHive ESP8266 firmware terminal in production
release with simple support of ansi escape codes
esp-flasher:
Simple tool for flashing DeviceHive firmware in ESP8266

# How To Build
run 'make'
all binary files will be generated in 'build' directory.

# esp-terminal usage
Run application and it will try to detect device automatically. You also can
specify device manually by passing port name in command line.
To quit from terminal press Ctrl+Q

# esp-flasher usage
Run application and it will try to detect device automatically. If no parameters
were specified it also will try to open files 0x00000.bin and 0x40000.bin in
current directory and flash them to corresponding addresses.
You can specify port name in first argument if you want to specify it manually.
You also can specify which files have to be written in devices in arguments by
pairs hex address <space> file name. For exmaple:
esp-flasher COM2 0x00000 boot.img 0x40000 spi.img
esp-flasher 0x40000 myimagefile.bin

# License
see LICENSE file

# Authors
Nikolay Khabarov
