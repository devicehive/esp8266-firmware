#!/bin/bash

# This util was written for ESP8266 Device Hive firmware
# But it can be used for any other projects
# Author Khabarov Nikolay
# License is MIT

set -e

if [ "$#" -lt 1 ]; then
  echo Util for generating binary firmware images for ESP8266
  echo Please specify compliled binary in arg
fi

TARGETELF=$1
BUILDDIR=$(dirname "$1")
FWFILE="devicehive.bin"
if [ "$#" -gt 1 ]; then
  FWFILE="$2"
fi

if [[ $(uname) == *"MINGW"* ]]; then
  CROSS_COMPILE="c:/Espressif/xtensa-lx106-elf/bin/xtensa-lx106-elf-"
else
  CROSS_COMPILE="/opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin/xtensa-lx106-elf-"
fi
OBJCOPY=${CROSS_COMPILE}objcopy
NM=${CROSS_COMPILE}nm

write_hex32() {
  echo -n -e  $(echo $1 | sed -E 's/(..)(..)(..)(..).*/\\x\4\\x\3\\x\2\\x\1/') >> $FWFILE
}

write_addr() {
  write_hex32 $($NM $TARGETELF | grep "$1")
}

file_size() {
  res=$(stat -c %s "$@" 2> /dev/null)
  if [ $? -ne 0 ]; then 
    res=$(stat -f %z "$@")
  fi
  echo $res
}

write_zeros() {
  printf %*s $1 | tr ' ' '\000' >> $FWFILE
}

write_section() {
  PAD=4
  FILESIZE=$(file_size "$@")
  PADSIZE=$(( ($PAD - $FILESIZE % $PAD) % $PAD ))
  BYTESWITHPAD=$(( $FILESIZE + $PADSIZE ))
  write_hex32 $(printf %08X $BYTESWITHPAD)
  cat "$@" >> $FWFILE
  write_zeros $PADSIZE
}

write_checksum() {
  PAD=16
  FILESIZE=$(( $(file_size $FWFILE) + 1))
  PADSIZE=$(( ($PAD - $FILESIZE % $PAD) % $PAD ))
  write_zeros $PADSIZE
  r=0xef;
  for c in $(od -vt u1 $@ | sed 's/[^ ]*//'); do 
    r=$(($c ^ $r))
  done
  echo -n -e $(printf "\\\x%02X" $r) >> $FWFILE
}

echo Extract sections...
$OBJCOPY --only-section .text -O binary $TARGETELF $BUILDDIR/text.bin
$OBJCOPY --only-section .data -O binary $TARGETELF $BUILDDIR/data.bin
$OBJCOPY --only-section .rodata -O binary $TARGETELF $BUILDDIR/rodata.bin
$OBJCOPY --only-section .irom0.text -O binary $TARGETELF $BUILDDIR/irom0.bin

echo Writing header...
# image header description
# first byte - magic - 0xE9, second number of segments, we have 3
# third SPI flash interface - 0x00: QIO, 0x01: QOUT, 0x02: DIO, 0x03: DOUT
# forth, high word flash size - 0x00: 512KB, 0x10: 256KB, 0x20: 1MB, 0x30: 2MB, 0x40: 4MB
# forth, low word CPU spped - 0x00: 40MHz, 0x01: 26MHz, 0x02: 20MHz, 0x0f: 80MHz
echo -n -e "\xE9\x03\x00\x00" >  $FWFILE
write_addr " call_user_start$"

echo Writing .text section...
write_addr " _text_start$"
write_section $BUILDDIR/text.bin

echo Writing .data section...
write_addr " _data_start$"
write_section $BUILDDIR/data.bin

echo Writing .rodata section...
write_addr " _rodata_start$"
write_section $BUILDDIR/rodata.bin

echo Writing checksum...
write_checksum $BUILDDIR/text.bin $BUILDDIR/data.bin $BUILDDIR/rodata.bin

echo Writing irom0 section...
write_zeros $(( 65536 - $(file_size $FWFILE)))
cat $BUILDDIR/irom0.bin >> $FWFILE

echo Done
echo "Firmware file is $FWFILE, size is $(file_size $FWFILE) bytes"
