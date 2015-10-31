#!/bin/bash

set -e

DIR=$(realpath $(dirname $0))
VER=$(grep FIRMWARE_VERSION firmware-src/sources/user_config.h | cut -d'"' -f2)
DSTDIR=$DIR/../dh-esp-firmware-v$VER

mkdir -p $DSTDIR
rm -rf $DSTDIR/*

cp $DIR/examples $DSTDIR -r
cp $DIR/release/utils/* $DSTDIR
cp $DIR/*.pdf $DSTDIR
(cd $DIR/firmware-src && make clean all) && cp $DIR/firmware-src/firmware/* $DSTDIR
zip -r -q $DSTDIR/../dh-esp-firmware-v$VER.zip $DSTDIR
tar -czf $DSTDIR/../dh-esp-firmware-v$VER.tar.gz $DSTDIR


