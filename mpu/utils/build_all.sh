#!/bin/bash
set -e 
ModemType=${1:-"ag35ce"}
GnssType=${2:-"ag35ce"}
WifiType=${3:-"None"}
HSMType=${4:-"None"}

echo """Start Build Servers    
       ModemType=${ModemType}
       GnssType=${GnssType} 
       WifiType=${WifiType}  
       HSMType=${HSMType}  
---------------------------------
"""

if [ -z "$CC" ];then
    . /opt/ql-ol-sdk/ql-ol-crosstool/ql-ol-crosstool-env-init
fi

ROOT_DIR=`cd $(dirname $0) && pwd`
INSTALL_PREFIX="${ROOT_DIR}/utils-out/"

## create install directory
if [ -d ${INSTALL_PREFIX} ]; then
    rm -rf ${INSTALL_PREFIX}
fi
mkdir -p ${INSTALL_PREFIX}

## cmake build
cd ${ROOT_DIR}
if [ -d build ];then
    rm -rf build
fi
mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DMODEMTYPE=${ModemType}
make -j4 install
