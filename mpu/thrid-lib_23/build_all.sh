#!/bin/bash
set -e
#. /opt/ql-ol-sdk/ql-ol-crosstool/ql-ol-crosstool-env-init

ROOT_DIR=`cd $(dirname $0) && pwd`
HOST=$(echo ${CC} | sed 's/-gcc.*//')
INSTALL_PREFIX="${ROOT_DIR}/thrid-lib-out/"

if [ ! -d ${INSTALL_PREFIX} ]; then
    mkdir -p ${INSTALL_PREFIX}
fi

dir_list=`find ${ROOT_DIR}/* -maxdepth 0 -type d | grep -v 'thrid-lib-out'`
for dir in ${dir_list}
do
    echo "============>>>>>> start build ${dir} "
    cd ${dir}
    . ./build.sh
done
