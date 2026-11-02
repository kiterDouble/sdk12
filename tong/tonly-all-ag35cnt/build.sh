#!/bin/bash
set -e

RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[0;33m"
BLUE="\033[0;34m"
RESET="\033[0m"

MPU_FW_PATH=$1
MCU_FW_PATH=$2

MPU_VER=$3
MCU_VER=$4

SDK_PATH=/opt/ql_crosstools/ql-ol-extsdk-quectel_ag35-xxt_r01a05_sdk
MPU_APP_PATH=/app_work
MPU_IMAGE_PATH=/mpu_image/$MPU_FW_PATH
MCU_IMAGE_PATH=/mcu_image/$MCU_FW_PATH

PACK_VER=${MPU_VER}${MCU_VER}_$(date +"%Y%m%d_%H%M%S")
OUTPUT_PATH=${MPU_APP_PATH}/mkimage/${PACK_VER}

SIRUN_OEMAPP_PATH=$MPU_APP_PATH/oemapp
SDK_OEMAPP_PATH=${SDK_PATH}/oemapp

SIRUN_C3_SERVER_PATH=$MPU_APP_PATH/C3_base_line/c3server
SIRUN_C3_OUT_PATH=$MPU_APP_PATH/C3_base_line/C3BaseLineOut
SIRUN_FOTA_PATH=$MPU_APP_PATH/fota
SIRUN_APP_PATH=$MPU_APP_PATH/app



# 更新cmake交叉编译环境
source /opt/ql_crosstools/crosstool-env-init

# 删除cetlib下不必要的库
# <!-- 删除/opt/ql_crosstools/cetlib/lib/目录下的ssl和crypto的1.0.0版本库 -->
rm -rf /opt/ql_crosstools/cetlib/lib/libssl*
rm -rf /opt/ql_crosstools/cetlib/lib/libcrypto*
# <!-- 删除/opt/ql_crosstools/cetlib/include/目录下的openssl文件夹 -->
rm -rf /opt/ql_crosstools/cetlib/include/openssl
# <!-- 删除/opt/ql_crosstools/cetlib/bin/openssl -->
rm -rf /opt/ql_crosstools/cetlib/bin/openssl


CompileC3Server()
{
    echo -e "${YELLOW}=============================== Start compile c3 server ==============================${RESET}"
    cd ${SIRUN_C3_SERVER_PATH}
    echo "${SIRUN_C3_SERVER_PATH}"

    /bin/bash ${SIRUN_C3_SERVER_PATH}/build_all.sh ag35cet

    if [ $? != 0 ];then
        echo 
        echo -e "${RED}============================= Compile server c3 FAILED ============================${RESET}"
        exit -1    
    fi
    # 拷贝server-out到C3BaseLineOut目录下
    # mkdir -p ${SIRUN_C3_OUT_PATH}/server-out/
    rm -rf ${SIRUN_C3_OUT_PATH}/server-out/*
    cp -r -d ${SIRUN_C3_SERVER_PATH}/server-out/ ${SIRUN_C3_OUT_PATH}/
    echo -e "${GREEN}============================= Compile server c3 SUCCEED ==============================${RESET}"
    echo
}

CompileThirdLib()
{
    echo -e "${YELLOW}=============================== Start compile third lib ==============================${RESET}"
    echo ">>> copy third lib"
    mkdir -p ${SIRUN_OEMAPP_PATH}/third_lib/ 
    cp -r -d ${MPU_APP_PATH}/app-third-lib/lib/*   ${SIRUN_OEMAPP_PATH}/third_lib/ 
    echo -e "${GREEN}============================= Compile third lib SUCCEED ==============================${RESET}"
    echo
}

CompileApp()
{
    echo -e "${YELLOW}================================ Start compile app ================================${RESET}"
    cd ${SIRUN_APP_PATH}
    echo "${SIRUN_APP_PATH}"
    /bin/bash ${SIRUN_APP_PATH}/build_all.sh ag35cet
    if [ $? != 0 ];then
        echo
        echo -e "${RED}============================= Compile app FAILED ============================${RESET}"
        exit -1
    fi
    echo -e "${GREEN}=============================== Compile app SUCCEED ===============================${RESET}"
    echo
}

CompileFotaDaemon()
{
    echo -e "${YELLOW}================================ Start compile fota daemon ================================${RESET}"
    cd ${SIRUN_FOTA_PATH}
    echo "${SIRUN_FOTA_PATH}"
    /bin/bash ${SIRUN_FOTA_PATH}/ota_daemon/build_all.sh ag35cet
    if [ $? != 0 ];then
        echo 
        echo -e "${RED}============================= Compile fota daemon FAILED ============================${RESET}"
        exit -1    
    fi
    echo -e "${GREEN}=============================== Compile fota daemon SUCCEED ===============================${RESET}"
    echo 
}

UpdateOemapp()
{
    echo -e "${YELLOW}============================ Start update oemapp ============================${RESET}"
    mkdir -p ${SIRUN_OEMAPP_PATH}/lib/
    mkdir -p ${SIRUN_OEMAPP_PATH}/servers/
    mkdir -p ${SIRUN_OEMAPP_PATH}/apps/
    mkdir -p ${SIRUN_OEMAPP_PATH}/demo/
    rm -rf ${SIRUN_OEMAPP_PATH}/lib/*
    rm -rf ${SIRUN_OEMAPP_PATH}/servers/*
    rm -rf ${SIRUN_OEMAPP_PATH}/apps/*
    rm -rf ${SIRUN_OEMAPP_PATH}/demo/*

    echo ">>> update lib"
    cp -r -d ${SIRUN_C3_OUT_PATH}/server-out/lib/* ${SIRUN_OEMAPP_PATH}/lib/
    cp -r -d ${SIRUN_C3_OUT_PATH}/thrid-lib-out/lib/* ${SIRUN_OEMAPP_PATH}/lib/
    cp -r -d ${SIRUN_C3_OUT_PATH}/utils-out/lib/* ${SIRUN_OEMAPP_PATH}/lib/

    echo ">>> update servers"
    cp -r -d ${SIRUN_C3_OUT_PATH}/server-out/server/*  ${SIRUN_OEMAPP_PATH}/servers/
    cp -r -d ${SIRUN_C3_OUT_PATH}/server-out/demo/*  ${SIRUN_OEMAPP_PATH}/demo/

    echo ">>> update apps"
    cp -r -d ${SIRUN_FOTA_PATH}/ota_daemon/out/*      ${SIRUN_OEMAPP_PATH}/apps/
    cp -r -d ${SIRUN_APP_PATH}/out/apps/*         ${SIRUN_OEMAPP_PATH}/apps/
    cp -r -d ${SIRUN_APP_PATH}/out/lib/*         ${SIRUN_OEMAPP_PATH}/lib/
    cp -r -d ${SIRUN_APP_PATH}/out/demo/*         ${SIRUN_OEMAPP_PATH}/demo/

    echo ">>> update config"
    cp -r -d ${SIRUN_APP_PATH}/out/ext_config/*  ${SIRUN_OEMAPP_PATH}/config/

# update build info
    mkdir -p ${SIRUN_OEMAPP_PATH}/version/
    echo `date +"%Y-%m-%d %H:%M:%S (UTC%::z)"`   > ${SIRUN_OEMAPP_PATH}/version/BUILD_INFO
    echo ${MPU_VER}                               > ${SIRUN_OEMAPP_PATH}/version/MPU_VERSION
    echo ${MCU_VER}                               > ${SIRUN_OEMAPP_PATH}/version/MCU_VERSION
    echo "${MPU_VER}${MCU_VER}"                   > ${SIRUN_OEMAPP_PATH}/version.txt


}

BuildOemappDisk()
{
    echo -e "${YELLOW}============================ Start Make oemapp.squashfs ============================${RESET}"

    cp ${SIRUN_OEMAPP_PATH}/* ${SDK_OEMAPP_PATH}/ -a

    cd $SDK_PATH

    make oemapp 
    if [ $? != 0 ];then
        echo -e "${RED}========================== make oemapp.squashfs failed ==========================${RESET}"
        exit -1    
    fi
    sync 

    echo -e "${YELLOW}========================= Make oemapp.squashfs succeed ==========================${RESET}"

    ls -l ${SDK_PATH}/target/oemapp.squashfs
}

MakeMPUImage()
{
    echo -e "${YELLOW}============================ Start MakeMPUImage =========================${RESET}"

    mkdir -p ${OUTPUT_PATH}/MPU/
    cp -rf ${MPU_IMAGE_PATH}/*                                              ${OUTPUT_PATH}/MPU/
    cp -f  ${SDK_PATH}/target/oemapp.squashfs                               ${OUTPUT_PATH}/MPU/
    cp -f  ${SDK_PATH}/tools/fota/gen_fota_image.sh                         ${OUTPUT_PATH}/MPU/
    cp -f  ${SDK_PATH}/tools/fota/tim_builder                               ${OUTPUT_PATH}/MPU/
    cp -f  ${SDK_PATH}/tools/fota/mkotafbf                                  ${OUTPUT_PATH}/MPU/
    cp -f  ${SDK_PATH}/tools/fota/quectel_AB_OTA.blf                        ${OUTPUT_PATH}/MPU/
    cp -f  ${SDK_PATH}/tools/fota/quectel_skylark_pm802_standard_AB.blf     ${OUTPUT_PATH}/MPU/
    cp -f  ${SDK_PATH}/tools/fota/mversion                                  ${OUTPUT_PATH}/MPU/

    chmod 755 ${OUTPUT_PATH}/MPU/gen_fota_image.sh

    echo -e "${YELLOW}========================= Make MakeMPUImage succeed ==========================${RESET}"
}

MakeMCUImage()
{
    echo -e "${YELLOW}============================ Start MakeMCUImage =========================${RESET}"
    mkdir -p ${OUTPUT_PATH}/MCU/
    cp -rf ${MCU_IMAGE_PATH}/*                                              ${OUTPUT_PATH}/MCU/
    echo -e "${YELLOW}========================= Make MakeMCUImage succeed ==========================${RESET}"
}

MakeOta()
{

    OTA_PARTITIONS=$1

    echo -e "${YELLOW}============================ Start make ota package ===${OTA_PARTITIONS}=========================${RESET}"
    mkdir -p ${OUTPUT_PATH}/FOTA/

    # step1、生成MPU侧FOTA固件包
    cd ${OUTPUT_PATH}/MPU/
    cp -rf ${OUTPUT_PATH}/MPU/TLoader_QSPINAND_ProductBuild.bin             ${OUTPUT_PATH}/MPU/TLoader_QSPINAND.bin
    cp -rf ${OUTPUT_PATH}/MPU/update.blf                                    ${OUTPUT_PATH}/MPU/quectel_skylark_pm802_standard_AB.blf

    # cat $OUTPUT_PATH/MPU/output_key.blf
    # 将quectel_AB_OTA.blf中的密钥对替换为烧录包使用的output_key.blf
    awk -v replfile="${OUTPUT_PATH}/MPU/output_key.blf" '
        /^End_Extended_Reserved_Data/ {
            print;
            while (getline line < replfile)
                print line;
            close(replfile);
            flag = 1;
            next;
        }
        flag && /^\[Image_List\]/ {
            print;
            flag = 0;
            next;
        }
        !flag
    ' "${OUTPUT_PATH}/MPU/quectel_AB_OTA.blf" > tmp && mv tmp "${OUTPUT_PATH}/MPU/quectel_AB_OTA.blf"

    if [ "$OTA_PARTITIONS" = "single" ]; then
        echo "single partition package packaging"
        bash ${OUTPUT_PATH}/MPU/gen_fota_image.sh oemapp > /dev/null 2>&1
    else
        echo "full package packaging"
        bash ${OUTPUT_PATH}/MPU/gen_fota_image.sh > /dev/null 2>&1
    fi

    if [ $? != 0 ];then
        echo -e "${RED}=====make ota package failured========================================${RESET}" 
        exit -1
    fi

    ls -lh ${OUTPUT_PATH}/MPU/quectel_AB_OTA.img
    mv -f ${OUTPUT_PATH}/MPU/quectel_AB_OTA.img  ${OUTPUT_PATH}/FOTA/update.zip


    # step2、MCU和MPU升级包MD5值生成、打包
    MCU1_FILE=$(ls "${OUTPUT_PATH}/MCU/mcu_1"*.bin 2>/dev/null | head -n 1)
    MCU2_FILE=$(ls "${OUTPUT_PATH}/MCU/mcu_2"*.bin 2>/dev/null | head -n 1)
    # 判断文件是否存在 
    if [ -f "$MCU1_FILE" ] && [ -f "$MCU2_FILE" ]; then 
        cp -f "${MCU1_FILE}"  "${OUTPUT_PATH}/FOTA/mcu_1.bin" 2>/dev/null || true
        cp -f "${MCU1_FILE}"  "${OUTPUT_PATH}/FOTA/mcu_2.bin" 2>/dev/null || true
    else 
        echo "mcu_1*.bin、mcu_2*.bin not found,exit."
        exit 1
    fi

    echo -e "${YELLOW}========================= Make ota package succeed ==========================${RESET}"


    echo "$PACK_VER" > ${OUTPUT_PATH}/FOTA/version.txt

    md5sum ${OUTPUT_PATH}/FOTA/update.zip    >  ${OUTPUT_PATH}/FOTA/md5sum.txt
    md5sum ${OUTPUT_PATH}/FOTA/mcu_1.bin    >>  ${OUTPUT_PATH}/FOTA/md5sum.txt
    md5sum ${OUTPUT_PATH}/FOTA/mcu_2.bin    >>  ${OUTPUT_PATH}/FOTA/md5sum.txt
    md5sum ${OUTPUT_PATH}/FOTA/version.txt  >>  ${OUTPUT_PATH}/FOTA/md5sum.txt

	cd   ${OUTPUT_PATH}/FOTA/
	zip -r ../update_${PACK_VER}.bin  ./*
	rm ./* -rf
	mv ../update_${PACK_VER}.bin ./
}

CompileC3Server

CompileThirdLib

CompileFotaDaemon

CompileApp

UpdateOemapp

BuildOemappDisk

MakeMPUImage

MakeMCUImage

MakeOta full










