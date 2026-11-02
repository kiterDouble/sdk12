#/usr/bin/bash

#INSTALL_PREFIX=/home/smbuser/test/thrid-lib-out/
rm -rf build
mkdir build
cd build
if [ "${HOST}" = "arm-openwrt-linux-muslgnueabi" ]; then
    cmake -DBOOST_ROOT=${INSTALL_PREFIX} -DBOOST_INCLUDEDIR=${INSTALL_PREFIX}include -DBOOST_LIBRARYDIR=${INSTALL_PREFIX}lib -DENABLE_SIGNAL_HANDLING=1 -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DCHIPA7805=ON ..
else
    cmake -DBOOST_ROOT=${INSTALL_PREFIX} -DBOOST_INCLUDEDIR=${INSTALL_PREFIX}include -DBOOST_LIBRARYDIR=${INSTALL_PREFIX}lib -DENABLE_SIGNAL_HANDLING=1 -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} ..
fi

make -j2
make install
make clean
