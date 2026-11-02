#!/bin/sh
partition()
{
fdisk /dev/mmcblk0	<< EOF
n
p
1

+1G
n
p
2

+1G
n
p
3

+11G
n
p


w
EOF
}

delpartition()
{
fdisk /dev/mmcblk0	<< EOF
d
1
d
2
d
3
d
w
EOF
}


mountEmmc()
{
	devNum="$1"
	mountDir="$2"
	SEARCH_DIR="$3"

    umount ${devNum}
	fsck.ext4 -y ${devNum}
	mount -t ext4 -o rw ${devNum} ${mountDir}
	ret=$?
	if [ $ret -ne 0 ];then
		echo "mount error ========"
		mkfs.ext4 ${devNum}
		umount ${devNum}
		mount -t ext4 -o rw ${devNum} ${mountDir}
		ret=$?
		if [ $ret -ne 0 ];then
			echo "mount always error ${devNum} ${mountDir} " >> /oemdata/log/sirunlog
		else
			echo "after mkfs mount  ${devNum} ${mountDir} successful" >> /oemdata/log/sirunlog
		fi
	else
		echo "emmc mount  ${devNum} ${mountDir} successful " >> /oemdata/log/sirunlog
	fi
}


#if Not found mmcblk0p1 and then partition
if [ `fdisk -l /dev/mmcblk0 | grep mmcblk0p | wc -l ` -ne 4 ];then
#	logger -t ${0} -p local7.info "partition mmc"
	umount /dev/mmcblk0p1
	delpartition
	echo "partition mmc"
	partition
	mkfs.ext4 -F /dev/mmcblk0p1
	mkfs.ext4 -F /dev/mmcblk0p2
	mkfs.ext4 -F /dev/mmcblk0p3
	mkfs.ext4 -F /dev/mmcblk0p4
fi


mountEmmc /dev/mmcblk0p1 /mnt/emmc/
mountEmmc /dev/mmcblk0p2 /mnt/srlog/
mountEmmc /dev/mmcblk0p3 /mnt/canlog/
mountEmmc /dev/mmcblk0p4 /mnt/carota/