#!/usr/bin/bash

if [ -d ./build ];then
    rm ./build -rf
fi

mkdir -p ./build/
cd ./build/
cmake .. -DBUILD_SHARED_LIBS=ON -DFLATCC_RTONLY=on -DFLATCC_INSTALL=on -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}
make -j4 install
