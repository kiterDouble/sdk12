#!/bin/sh
partition()
{
fdisk /dev/mmcblk0	<< EOF
n
p
1


w
EOF
}


#if Not found mmcblk0p1 and then partition
if [ `ls /dev/mmcblk0* | grep mmcblk0p1 | wc -l ` -eq 0 ];then
	logger -t ${0} -p local7.info "partition mmc"
	partition
	mkfs.ext4 -y /dev/mmcblk0p1
fi

sleep 0.1

mountDir=$1

if [ -d ${mountDir} ];then
	umount ${mountDir}
	timeout -t 10 fsck.ext4 -a /dev/mmcblk0p1
	mount -t ext4 -o rw /dev/mmcblk0p1 ${mountDir}
	ret=$?
	if [ $ret -ne 0 ];then
		echo "mount error ========"
		logger -t ${0} -p local7.info "mount emmc error first time "
		umount ${mountDir}
		mkfs.ext4 -y /dev/mmcblk0p1
		mount -t ext4 -o rw /dev/mmcblk0p1 ${mountDir}
		ret=$?
		if [ $ret -ne 0 ];then
			echo "mount always error and to partition"
			logger -t ${0} -p local7.info "mount emmc error second time "
			partition
			mkfs.ext4 -y /dev/mmcblk0p1
			mount -t ext4 -o rw /dev/mmcblk0p1 ${mountDir}
		else
			echo "after mkfs mount successful"
			logger -t ${0} -p local7.info "mount emmc success second time"
		fi
	else
		echo "mount successful "
		logger -t ${0} -p local7.info "mount emmc success first time "
	fi

fi

