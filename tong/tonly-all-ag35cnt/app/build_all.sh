#!/bin/bash
### bash build_all.sh ModemType GnssType WifiType HSMType
### 
set -e 
ModemType=${1:-"ag35cet"}
GnssType=${2:-"ag35cet"}
WifiType=${3:-"None"}
HSMType=${4:-"None"}

echo """Start Build App    
       ModemType=${ModemType}
       GnssType=${GnssType} 
       WifiType=${WifiType}  
       HSMType=${HSMType}  
---------------------------------
"""

if [ -z "$CC" ];then
    . /opt/ql_crosstools/crosstool-env-init
fi

ROOT_DIR=`cd $(dirname $0) && pwd`
INSTALL_PREFIX="${ROOT_DIR}/out/"

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

## strip installed binaries to reduce size
if [ -n "$STRIP" ] && command -v "$STRIP" >/dev/null 2>&1; then
    echo "Stripping installed binaries with: $STRIP"
    find "${INSTALL_PREFIX}" -type f \( -name "*.so*" -o -name "*.a" -o -perm /111 \) -exec "$STRIP" {} + 2>/dev/null || true
else
    echo "Warning: strip tool not found, skipping install strip"
fi

