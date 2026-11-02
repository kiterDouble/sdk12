#!/bin/bash

VaildProject="AG35-CETCA AG35-CETDA AG35-EUTAA AG35-EUTDA AG35-EUTDAM AG35-JPTDA AG35-LATDA AG35-CNTCA AG35-CNTDA AG35-CNTDAM"

# Customer config
FEATURE_WIFI_FC908A=n
FEATURE_WIFI_FCS950U=n
FEATURE_WIFI_FCS960K=n
FEATURE_WIFI_FC20=n
FEATURE_WIFI_AF20=n
FEATURE_GDB=n
FEATURE_SSH=n

function help() 
{
	echo
	echo -e "\033[33m==============================================================================\033[0m"
	echo -e "\033[33m                               AG35 SDK build                                 \033[0m"
	echo
	echo -e "\033[33mUsage:\033[0m"
	echo -e "\033[33m  ./build_all_images.sh <Project>\033[0m"
	echo

	echo -e "\033[33mParams:\033[0m"
	echo -e "\033[33m  Valid Project  :  $VaildProject\033[0m"
	echo

	echo -e "\033[33mProjects:\033[0m"
	echo -e "\033[33m  1. AG35-CET(CA)/AG35-CET(AA) : AG35-CETCA\033[0m"
	echo -e "\033[33m  2. AG35-CET(DA)  : AG35-CETDA\033[0m"
	echo -e "\033[33m  3. AG35-EUT(DA)  : AG35-EUTDA\033[0m"
	echo -e "\033[33m  4. AG35-EUT(AA)  : AG35-EUTAA\033[0m"
	echo -e "\033[33m  5. AG35-EUT(DAM) : AG35-EUTDAM\033[0m"
	echo -e "\033[33m  6. AG35-JPT(DA)  : AG35-JPTDA\033[0m"
	echo -e "\033[33m  7. AG35-LAT(DA)  : AG35-LATDA\033[0m"
	echo -e "\033[33m  8. AG35-CNT(DA)  : AG35-CNTDA\033[0m"
	echo -e "\033[33m  9. AG35-CNT(DAM) : AG35-CNTDAM\033[0m"
	echo -e "\033[33m 10. AG35-CNT(CA)  : AG35-CNTCA\033[0m"
	echo

	echo -e "\033[33mExample:\033[0m"
	echo -e "\033[33m  ./build_all_images.sh AG35-CETCA\033[0m"
	echo

	echo -e "\033[32;1m*******************************CURRENT CONFIG******************************** \033[0m"
	echo -e "\033[32;1mFEATURE_WIFI_FC908A  = ${FEATURE_WIFI_FC908A} \033[0m"
	echo -e "\033[32;1mFEATURE_WIFI_FCS950U = ${FEATURE_WIFI_FCS950U} \033[0m"
	echo -e "\033[32;1mFEATURE_WIFI_FCS960K = ${FEATURE_WIFI_FCS960K} \033[0m"
	echo -e "\033[32;1mFEATURE_WIFI_FC20    = ${FEATURE_WIFI_FC20} \033[0m"
	echo -e "\033[32;1mFEATURE_WIFI_AF20    = ${FEATURE_WIFI_AF20} \033[0m"
	echo -e "\033[32;1mFEATURE_GDB          = ${FEATURE_GDB} \033[0m"
	echo -e "\033[32;1mFEATURE_SSH          = ${FEATURE_SSH} \033[0m"
	echo

	echo -e "\033[33m==============================================================================\033[0m"
	echo
}

# Check valid input params
if [ "$1" = "" ]; then
	help && exit -1
fi

for ver in $VaildProject;do
	if [ $1 = $ver ];then
		PROJECT_NAME=$1
		break
	fi
done

if [ "$PROJECT_NAME" != "$1" ];then
	echo -e "\033[31;1mInvalid project name: $1!\033[0m"
	echo -e "\033[31;1mPlease Enter Valid ProjectName(${VaildProject})\033[0m"
	help
	exit -1
fi

# Build prepare
SDK_DIR="$(pwd)"
UBOOT_DIR=${SDK_DIR}/ql-ol-bootloader
ROOTFS_DIR=${SDK_DIR}/ql-ol-rootfs
KERNEL_DIR=${SDK_DIR}/ql-ol-kernel
TARGET_DIR=${SDK_DIR}/target/${PROJECT_NAME}
OEMAPP_DIR=${SDK_DIR}/oemapp
OEMDATA_DIR=${SDK_DIR}/oemdata
IMAGE_DIR=${SDK_DIR}/images
FEATURE_DIR=${SDK_DIR}/utils

mkdir -p ${TARGET_DIR};
mkdir -p ${OEMAPP_DIR};
if [ ! -d ${ROOTFS_DIR} ]; then
	echo "Extracting the rootfs..."
	mkdir -p ${ROOTFS_DIR}
	tar -xf ${SDK_DIR}/ql-ol-rootfs.tar.gz -C ${ROOTFS_DIR}
fi

PROJECT_VERSION=`cat ${IMAGE_DIR}/autogen/version_info.txt | grep "^${PROJECT_NAME}," | awk -F "," '{print $2}' | tr -d " "`
PROJECT_NAME_NO_DASH=$(echo ${PROJECT_NAME} | sed 's/-//')
SDK_VERSION=`cat ${IMAGE_DIR}/autogen/quectel-sdk-version | grep 'Quectel_AG35-XXT' | awk -F ":" '{print $2}' | tr -d " "`

# include project config
source ${IMAGE_DIR}/config/Project_${PROJECT_NAME}.conf

# Generate /etc/quectel-project-version baseon project
rm -f ${ROOTFS_DIR}/etc/quectel-project-version
echo "Project Name: ${SDK_VERSION}_${PROJECT_NAME}" > ${ROOTFS_DIR}/etc/quectel-project-version
echo "Project Rev : ${PROJECT_VERSION}" >> ${ROOTFS_DIR}/etc/quectel-project-version
echo "Build   Date: " >> ${ROOTFS_DIR}/etc/quectel-project-version

# Generate build config files base on project
rm -f ${UBOOT_DIR}/include/quectel_build_config.h
echo "#ifndef __QL_BUILDCONFIG_GEN_H__" > ${UBOOT_DIR}/include/quectel_build_config.h
echo "#define __QL_BUILDCONFIG_GEN_H__" >> ${UBOOT_DIR}/include/quectel_build_config.h
echo "#define QUECTEL_PROJECT_NAME \"${PROJECT_NAME}\"" >> ${UBOOT_DIR}/include/quectel_build_config.h
echo "#endif" >> ${UBOOT_DIR}/include/quectel_build_config.h

# Generate project mk
echo "PROJECT_NAME:=${PROJECT_NAME}" > ${SDK_DIR}/project.mk

# Instll utils
if [ ${FEATURE_WIFI_FC908A} == "y" ];then
	echo "Install WIFI FC908A driver and firmware ..."
	cp -rf ${FEATURE_DIR}/wifi/FC908A/* ${ROOTFS_DIR}
fi

if [ ${FEATURE_WIFI_FCS950U} == "y" ];then
	echo "Install WIFI FCS950U driver and firmware ..."
	cp -rf ${FEATURE_DIR}/wifi/FCS950U/* ${ROOTFS_DIR}
fi

if [ ${FEATURE_WIFI_FCS960K} == "y" ];then
	echo "Install WIFI FCS960K driver and firmware ..."
	cp -rf ${FEATURE_DIR}/wifi/FCS960K/* ${ROOTFS_DIR}
fi

if [ ${FEATURE_WIFI_FC20} == "y" ];then
	echo "Install WIFI FC20 driver and firmware ..."
	cp -rf ${FEATURE_DIR}/wifi/FC20/* ${ROOTFS_DIR}
fi

if [ ${FEATURE_WIFI_AF20} == "y" ];then
	echo "Install WIFI AF20 driver and firmware ..."
	cp -rf ${FEATURE_DIR}/wifi/AF20/* ${ROOTFS_DIR}
fi

if [ ${FEATURE_GDB} == "y" ];then
	echo "Install gdb ..."
	cp -rf ${FEATURE_DIR}/gdb/* ${ROOTFS_DIR}
fi

if [ ${FEATURE_SSH} == "y" ];then
	echo "Install sshd ..."
	cp -rf ${FEATURE_DIR}/sshd/* ${ROOTFS_DIR}
fi

# Copy prebuild images
rm -rf ${TARGET_DIR}/*
cp -rf ${IMAGE_DIR}/${PROJECT_NAME}_RFPLUGIN.bin ${TARGET_DIR}/RFPLUGIN.bin
cp -rf ${IMAGE_DIR}/ARBEL.bin ${TARGET_DIR}/
cp -rf ${IMAGE_DIR}/MSA.bin ${TARGET_DIR}/
cp -rf ${IMAGE_DIR}/hotfix.bin ${TARGET_DIR}/
cp -rf ${IMAGE_DIR}/TLoader_QSPINAND.bin ${TARGET_DIR}/
cp -rf ${IMAGE_DIR}/TLoader_QSPINAND_ProductBuild.bin ${TARGET_DIR}/
cp -rf ${IMAGE_DIR}/tos.bin ${TARGET_DIR}/

# Copy dbg
cp -rf ${IMAGE_DIR}/dbg ${TARGET_DIR}/

# Copy update blf
cp -rf ${IMAGE_DIR}/quectel_1806e_update.blf ${TARGET_DIR}/update.blf
if [ ${QUECTEL_NAND_SIZE} = '512' ];then
	sed -i 's/2_Eraseonly_Area_Size = 0x05000000/2_Eraseonly_Area_Size = 0x14340000/g' ${TARGET_DIR}/update.blf
	sed -i 's/1_NandSign_1_NandSize = 0x10000000/1_NandSign_1_NandSize = 0x20000000/g' ${TARGET_DIR}/update.blf
fi

# Copy fota blf
cp -f ${IMAGE_DIR}/quectel_1806e_update.blf ${SDK_DIR}/tools/fota/quectel_skylark_pm802_standard_AB.blf

# Copy gnss config files
if [ "${QUECTEL_GNSS_CHIP}" == "at6558f" ];then
	cp -f ${IMAGE_DIR}/config/ql_locationd_at6558f.conf ${ROOTFS_DIR}/etc/ql_locationd.conf
fi

if [ "${QUECTEL_GNSS_CHIP}" == "at6668f" ];then
	cp -f ${IMAGE_DIR}/config/ql_locationd_at6668f.conf ${ROOTFS_DIR}/etc/ql_locationd.conf
fi

if [ "${QUECTEL_GNSS_CHIP}" == "uc6228ci" ];then
	cp -f ${IMAGE_DIR}/config/ql_locationd_uc6228ci.conf ${ROOTFS_DIR}/etc/ql_locationd.conf
fi

if [ "${QUECTEL_GNSS_CHIP}" == "at9850" ];then
	cp -f ${IMAGE_DIR}/config/ql_locationd_at9850.conf ${ROOTFS_DIR}/etc/ql_locationd.conf
fi

################################################################################################################################
make pre_build
rm ${ROOTFS_DIR}/sdcard -rf 

# add sirun patch
if [ -d /work/sirun_patch/ql-ol-bootloader ]; then
	echo "add bootloader patch !!!"
	cp -a /work/sirun_patch/ql-ol-bootloader/*  ${UBOOT_DIR}/
fi

if [ -d /work/sirun_patch/ql-ol-kernel ]; then
	echo "add kernel patch !!!"
	cp -a /work/sirun_patch/ql-ol-kernel/*  ${KERNEL_DIR}/
fi

if [ -d /work/sirun_patch/ql-ol-rootfs ]; then
	echo "add rootfs patch !!!"
	cp -a /work/sirun_patch/ql-ol-rootfs/*  ${ROOTFS_DIR}/
fi

################################################################################################################################


# Build uboot
echo "Build uboot ..."
make uboot

# Build Kernel
echo "Build kernel ..."
make kernel
make kernel_module

# Build rootfs
echo "Build rootfs ..."
make rootfs

# Build oemapp
echo "Build oemapp ..."
make oemapp

# Build oemdata
echo "Build oemdata ..."
make oemdata

echo "All work done!"

################################################################################################################################
# copy output image
outputImagePath=/work/image/${PROJECT_NAME}_OS_$(date +"%Y%m%d_%H%M")
if [ ! -d ${outputImagePath} ]; then
	mkdir -p ${outputImagePath}
fi

cp $TARGET_DIR/*              ${outputImagePath}/ -a
cp ${KERNEL_DIR}/.config      ${outputImagePath}/

keyFilePath=/work/key/output_key.blf
cp ${keyFilePath}             ${outputImagePath}/

# 1.使能trusted boot（移远默认开启）
sed -i 's/^Trusted\s*=\s*[0-9]\+/Trusted = 1/' "${outputImagePath}/update.blf"
# 2.使能fuse熔断
awk '/^End_BBMT/ { print; print "FUSE"; print "Enabled = 0x00000001"; print "End_FUSE"; next } { print }' \
    "${outputImagePath}/update.blf" > tmp && mv tmp "${outputImagePath}/update.blf"
# 3.使能熔断obm（使用SDK自带的TLoader_QSPINAND_ProductBuild.bin）
sed -i -e 's/^\(3_Image_Path\s*=\s*\).*$/\1TLoader_QSPINAND_ProductBuild.bin/' \
       -e 's/^\(4_Image_Path\s*=\s*\).*$/\1TLoader_QSPINAND_ProductBuild.bin/' "${outputImagePath}/update.blf"
# 4.配置/覆盖update.blf中的密钥对改为output_key.blf
awk -v replfile="${outputImagePath}/output_key.blf" '
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
' "${outputImagePath}/update.blf" > tmp && mv tmp "${outputImagePath}/update.blf"
################################################################################################################################


