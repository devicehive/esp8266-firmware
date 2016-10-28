#!/bin/bash

set -e

DIR=$(realpath $(dirname $0))
BUILD=$DIR/build
VER=$(grep FIRMWARE_VERSION $DIR/../firmware-src/sources/user_config.h | cut -d'"' -f2)

mkdir -p $BUILD
rm -rf $BUILD/*
rm -f $BUILD/../dh-esp-firmware-v$VER.zip
rm -f $BUILD/../dh-esp-firmware-v$VER.tar.gz
(cd $DIR/../esp-utils && CXX=x86_64-linux-gnu-g++ CFLAGS=-m32 LDFLAGS=-m32 make rebuild)
cp $DIR/../esp-utils/build/esp-terminal $BUILD/esp-terminal-linux
cp $DIR/../esp-utils/build/esp-flasher $BUILD/esp-flasher-linux
(cd $DIR/../esp-utils && CXX=i686-apple-darwin10-g++ make rebuild)
cp $DIR/../esp-utils/build/esp-terminal $BUILD/esp-terminal-osx
cp $DIR/../esp-utils/build/esp-flasher $BUILD/esp-flasher-osx
(cd $DIR/../esp-utils && CXX=i686-w64-mingw32-g++ LDFLAGS=-static make rebuild)
cp $DIR/../esp-utils/build/esp-terminal $BUILD/esp-terminal-win.exe
cp $DIR/../esp-utils/build/esp-flasher $BUILD/esp-flasher-win.exe
cp  $DIR/utils/* $BUILD/

# since an issue - https://github.com/ariya/phantomjs/issues/13959 (markdown-pdf uses phantomjs)
# just remove the local links from document.
sed $DIR/../DeviceHiveESP8266.md -e 's/\[\([^]].*\)\](#.*)/\1/g' > $DIR/../DeviceHiveESP8266_hack.md
(cd $DIR/.. && markdown-pdf DeviceHiveESP8266_hack.md -o $BUILD/DeviceHiveESP8266.pdf --css-path $DIR/md.css)
rm $DIR/../DeviceHiveESP8266_hack.md

(cd $DIR/../firmware-src && make rebuild) && cp $DIR/../firmware-src/firmware/* $BUILD

(cd $BUILD && zip -r -q ../dh-esp-firmware-v$VER.zip *)
(cd $BUILD && tar -czf ../dh-esp-firmware-v$VER.tar.gz *)


