#!/bin/bash
if [[ $(uname) == *"MINGW"* ]]; then
  taskkill.exe //im putty.exe
  (cd "$( dirname "${BASH_SOURCE[0]}")" && C:/MinGW/bin/mingw32-make.exe flash) && "C:\Program Files (x86)\PuTTY\putty.exe" -serial COM2 -sercfg 115200,8,n,1,N
else
  killall screen
  (cd "$( dirname "${BASH_SOURCE[0]}" )" && make flash) && screen /dev/ttyUSB1 115200
fi
