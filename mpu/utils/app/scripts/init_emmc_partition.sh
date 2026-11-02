#!/bin/sh
partition()
{
fdisk /dev/mmcblk0	<< EOF
n
p
1

+3G
n
p
2

+500M
n
p
3

+3G
n
p
4


w
EOF
}


mountExt4Dir()
{
	emmcNum=$1
	mountDir=$2

	if [ -d ${mountDir} ];then
		umount /dev/mmcblk0p$emmcNum
		fsck.ext4 -y /dev/mmcblk0p$emmcNum
		mount -t ext4 -o rw /dev/mmcblk0p$emmcNum ${mountDir}
		mountRet=$?
		if [ $mountRet -ne 0 ];then
			echo "mount error ========"
	#		logger -t ${0} -p local7.info "mount emmc error first time "
			echo "mount emmc error first time " >> /oemdata/mountemmc.log
			umount /dev/mmcblk0p$emmcNum
			mkfs.ext4 /dev/mmcblk0p$emmcNum
			mount -t ext4 -o rw /dev/mmcblk0p$emmcNum ${mountDir}
			mountRet=$?
		else
			echo "emmc mount /dev/mmcblk0p$emmcNum successful " >> /oemdata/mountemmc.log
	#		logger -t ${0} -p local7.info "mount emmc success first time "
		fi
	fi
	return $mountRet
}

logLineNum=`cat /oemdata/mountemmc.log |wc -l`
if [ $logLineNum -gt 10000 ];then
	sed -i '1,100d' filename.txt
fi


#if Not found mmcblk0p1 and then partition
if [ `fdisk -l /dev/mmcblk0 | grep mmcblk0p1 | wc -l ` -eq 0 ];then
#	logger -t ${0} -p local7.info "partition mmc"
	echo "partition mmc" >> /oemdata/mountemmc.log
	partition
	mkfs.ext4 /dev/mmcblk0p1
	mkfs.ext4 /dev/mmcblk0p2
	mkfs.ext4 /dev/mmcblk0p3
	mkfs.ext4 /dev/mmcblk0p4
fi

usleep 100000
emmcMountDir=/mnt/emmc/
audioMountDir=/mnt/audio/
carOtaMountDir=/mnt/carota/
canlogMountDir=/mnt/canlog/

mountExt4Dir 1 $emmcMountDir
mountEmmcRet=$?
mountExt4Dir 2 $audioMountDir
mountAudioRet=$?
mountExt4Dir 3 $carOtaMountDir
mountCarOtaRet=$?
mountExt4Dir 4 $canlogMountDir
mountCanLogRet=$?

if [ $mountEmmcRet -ne 0 && $mountAudioRet -ne 0 && $mountCarOtaRet -ne 0 && $mountCanLogRet -ne 0 ];then
	partition
	mkfs.ext4 /dev/mmcblk0p1
	mkfs.ext4 /dev/mmcblk0p2
	mkfs.ext4 /dev/mmcblk0p3
	mkfs.ext4 /dev/mmcblk0p4
	mountExt4Dir 1 $emmcMountDir
	mountEmmcRet=$?
	mountExt4Dir 2 $emmcMountDir
	mountAudioRet=$?
	mountExt4Dir 3 $emmcMountDir
	mountCarOtaRet=$?
	mountExt4Dir 4 $emmcMountDir
	mountCanLogRet=$?
fi


