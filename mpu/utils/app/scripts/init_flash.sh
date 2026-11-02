#!/bin/sh 

ubiNum=28
ubiDevice=/dev/mtd28
spiNand=/dev/ubi1
spiNandNum=/dev/ubi1_0
ubifileNum=ubi1_0
mountPath=$1

InitFlashForce()
{
#	logger -t $0 -p local7.info "start force init flash"
 	echo "start force init flash" >> /oemdata/log/sirunlog
    umount ${mountPath}
    usleep 100000 
    ubidetach /dev/ubi_ctrl -m ${ubiNum}
	usleep 100000
	ubiformat ${ubiDevice} -y
	usleep 100000
	ubiattach /dev/ubi_ctrl -m ${ubiNum}
	usleep 100000
	ubimkvol ${spiNand} -s 121MiB -N giga_ubi_vol
#	logger -t $0 -p local7.info "complete force init flash"
	echo "complete force init flash" >> /oemdata/log/sirunlog
}

#check mtd device
repeatTimes=0
while [ ! -c "${ubiDevice}" ]
do
	repeatTimes=$(($repeatTimes+1))
	if [ $repeatTimes -gt 20 ];then 
#		logger -t $0 -p local7.info "there no flash"
		echo "there no flash" >> /oemdata/log/sirunlog
        SaveLogInner
		exit 1
	fi
#    logger -t $0 -p local7.info "flash ${ubiDevice} has no exist and slepp 100ms"
#	echo "flash ${ubiDevice} has no exist and slepp 100ms" >> /oemdata/log/sirunlog
	usleep 100000
done

#logger -t $0 -p local7.info "${ubiDevice} exist"
echo "${ubiDevice} exist"  >> /oemdata/log/sirunlog

#Check whether flash attached
if [ ! -c "${spiNand}" ];then
#	logger -t $0 -p local7.info  "${spiNand} hos no exist"
	echo "${spiNand} hos no exist"  >> /oemdata/log/sirunlog

	ubiattach /dev/ubi_ctrl -m ${ubiNum}
	
	if [ $? -ne 0 ];then 
#		logger -t $0 -p local7.info  "flash ubiattach error first time"
		echo "flash ubiattach error first time" >> /oemdata/log/sirunlog
        umount ${mountPath}
		ubidetach /dev/ubi_ctrl -m ${ubiNum}
		ubiformat ${ubiDevice} -y 
		usleep 100000
		ubiattach /dev/ubi_ctrl -m ${ubiNum}
		if [ ! $? -eq 0 ];then
#			logger -t $0 -p local7.info  "flash ubiattach error"
			echo   "flash ubiattach error"  >> /oemdata/log/sirunlog
#			SaveLogInner
			exit 1
		fi
	fi
fi

#logger -t $0 -p local7.info "$[spiNand] exist" 
echo "${spiNand} exist"  >> /oemdata/log/sirunlog
usleep 100000

#check whether flash  has made volum 
if [ ! -c "${spiNandNum}" ];then
#	logger -t $0 -p local7.info  " ${spiNandNum} has no exist"
	echo " ${spiNandNum} has no exist"  >> /oemdata/log/sirunlog
	
	ubimkvol ${spiNand} -s 121MiB -N giga_ubi_vol 
	if [ ! $? -eq 0 ];then
#		logger -t $0 -p local7.info  "flash ubimkvol error first time"
		echo "flash ubimkvol error first time"  >> /oemdata/log/sirunlog
        umount ${mountPath}
		ubidetach /dev/ubi_ctrl -m ${ubiNum}
		ubiformat ${ubiDevice} -y
		usleep 100000
		ubiattach /dev/ubi_ctrl -m ${ubiNum}
		usleep 100000
		ubimkvol ${spiNand} -s 121MiB -N giga_ubi_vol
		if [ ! $? -eq 0 ];then
#			logger -t $0 -p local7.info  "flash ubimkvol error"
#            echo "flash ubimkvol error"
#			SaveLogInner
			exit 1
		fi
	fi
fi

#logger -t $0 -p local7.info "${spiNandNum} exist"
echo "${spiNandNum} exist"  >> /oemdata/log/sirunlog

#mount flash devic:
mount -t ubifs ${ubifileNum} ${mountPath}
if [ ! $? -eq 0 ];then
#	logger -t $0 -p local7.info  "flash mount error first time "
    echo "flash mount error first time "  >> /oemdata/log/sirunlog
	InitFlashForce
    usleep 100000
    mount -t ubifs ubifileNum ${mountPath}
	if [ ! $? -eq 0 ];then
#		logger -t $0 -p local7.info  "flash mount error"
        echo "flash mount error"  >> /oemdata/log/sirunlog
#        killall log_process.sh
#		SaveLogInner
		exit 1
	fi
fi

#logger -t $0 -p local7.info  "flash mount success"
echo "flash mount success"  >> /oemdata/log/sirunlog
