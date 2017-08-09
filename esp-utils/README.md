# DeviceHive ESP8266 Simple utils
`esp-terminal`:
Simple tool for easy access DeviceHive ESP8266 firmware terminal in production
release with simple support of ansi escape codes

`esp-flasher`:
Simple tool for flashing DeviceHive firmware in ESP8266

# How To Build
Run `make`.
All binary files will be generated in `build` directory.

# esp-terminal usage
Run application and it will try to detect device automatically. You also can
specify device manually by passing port name in command line.
To quit from terminal press `Ctrl+Q`.

# esp-flasher usage
Run application and it will try to detect device automatically. If no parameters
were specified it also will try to open files `devicehive.bin` in current directory
and directory with its binary and flash them to corresponding addresses.
You can specify port name in first argument if you want to specify it manually.
You also can specify which files have to be written in devices in arguments by
pairs `hex address <space> file name`. For exmaple:

```
esp-flasher COM2 0x00000 boot.img 0x40000 spi.img
esp-flasher 0x40000 myimagefile.bin
```

There also `--developer` and `--reboot` arguments which supposed to be used by
developers only. First enables incremental flash mode, it compares previosuly
flashed file (should be saved as `devicehive.bin.prev`) and if differences are
minimal it will flash only them to save time on flashing. `--reboot` argument
simply reboot chip (serial adapter `RTS` should be connected to `GPIO0`, `DTR` to `RTS`
pin).

# License
See [LICENSE](./LICENSE) file.
