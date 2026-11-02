#!/usr/bin/bash

if [ "${HOST}" = "arm-openwrt-linux-muslgnueabi" ]; then
    ./configure --prefix=${INSTALL_PREFIX} --host=arm-linux --enable-static=no
elif [ "${HOST}" = "arm-poky-linux-gnueabi" ]; then
    ./configure --prefix=${INSTALL_PREFIX} --host=${HOST} --enable-static=no --disable-openssl
else
    ./configure --prefix=${INSTALL_PREFIX} --host=${HOST} --enable-static=no
fi

make
make install
make clean
make distclean