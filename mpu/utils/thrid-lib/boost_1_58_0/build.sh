#!/usr/bin/bash
sed -i 's/using gcc : arm : [^ ]*-gcc ;/using gcc ;/g' project-config.jam
USINGGCC=`echo "s/\\(using gcc ;\\)/using gcc : arm : $HOST-gcc ;/g"`
sed -i "${USINGGCC}" project-config.jam
./b2 --clean
if [ "${HOST}" = "arm-poky-linux-gnueabi" ]; then
    ./b2 -j2 -q --prefix=${INSTALL_PREFIX} --ignore-site-config --layout=system install cflags="-mfloat-abi=hard -mfpu=neon" cxxflags="-mfloat-abi=hard -mfpu=neon"
else
    ./b2 -j2 -q --prefix=${INSTALL_PREFIX} --ignore-site-config --layout=system install cflags="-mfloat-abi=hard" cxxflags="-mfloat-abi=hard"
fi
