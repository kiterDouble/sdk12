#!/bin/sh 

ubiNum=30
ubiDevice=/dev/mtd30
spiNand=/dev/ubi6
spiNandNum=/dev/ubi6_0
ubifileNum=ubi6_0
mountPath=$1

InitFlashForce()
{
	logger -t $0 -p local7.info "start force init flash"
    umount ${mountPath}
    sleep 0.1 
    ubidetach /dev/ubi_ctrl -m ${ubiNum}
	sleep 0.1
	ubiformat ${ubiDevice} -y
	sleep 0.1
	ubiattach /dev/ubi_ctrl -m ${ubiNum}
	sleep 0.1
	ubimkvol ${spiNand} -s 121MiB -N giga_ubi_vol
	logger -t $0 -p local7.info "complete force init flash"
}

#check mtd device
repeatTimes=0
while [ ! -c "${ubiDevice}" ]
do
	repeatTimes=$(($repeatTimes+1))
	if [ $repeatTimes -gt 20 ];then 
		logger -t $0 -p local7.info "there no flash"
		echo "there no flash"
        SaveLogInner
		exit 1
	fi
    logger -t $0 -p local7.info "flash ${ubiDevice} has no exist and slepp 100ms"
	echo "flash ${ubiDevice} has no exist and slepp 100ms"
	sleep 0.1
done

logger -t $0 -p local7.info "${ubiDevice} exist"
echo "${ubiDevice} exist" 

#Check whether flash attached
if [ ! -c "${spiNand}" ];then
	logger -t $0 -p local7.info  "${spiNand} hos no exist"
	echo "${spiNand} hos no exist"

	ubiattach /dev/ubi_ctrl -m ${ubiNum}
	
	if [ $? -ne 0 ];then 
		logger -t $0 -p local7.info  "flash ubiattach error first time"
		echo "flash ubiattach error first time"
        umount ${mountPath}
		ubidetach /dev/ubi_ctrl -m ${ubiNum}
		ubiformat ${ubiDevice} -y 
		sleep 0.1
		ubiattach /dev/ubi_ctrl -m ${ubiNum}
		if [ ! $? -eq 0 ];then
			logger -t $0 -p local7.info  "flash ubiattach error"
			echo   "flash ubiattach error"
			SaveLogInner
			exit 1
		fi
	fi
fi

logger -t $0 -p local7.info "$[spiNand] exist" 
echo "${spiNand} exist"
sleep 0.1

#check whether flash  has made volum 
if [ ! -c "${spiNandNum}" ];then
	logger -t $0 -p local7.info  " ${spiNandNum} has no exist"
	echo " ${spiNandNum} has no exist"
	
	ubimkvol ${spiNand} -s 121MiB -N giga_ubi_vol 
	if [ ! $? -eq 0 ];then
		logger -t $0 -p local7.info  "flash ubimkvol error first time"
		echo "flash ubimkvol error first time"
        umount ${mountPath}
		ubidetach /dev/ubi_ctrl -m ${ubiNum}
		ubiformat ${ubiDevice} -y
		sleep 0.1
		ubiattach /dev/ubi_ctrl -m ${ubiNum}
		sleep 0.1
		ubimkvol ${spiNand} -s 121MiB -N giga_ubi_vol
		if [ ! $? -eq 0 ];then
			logger -t $0 -p local7.info  "flash ubimkvol error"
            echo "flash ubimkvol error"
			SaveLogInner
			exit 1
		fi
	fi
fi

logger -t $0 -p local7.info "${spiNandNum} exist"
echo "${spiNandNum} exist"

#mount flash devic:
mount -t ubifs ${ubifileNum} ${mountPath}
if [ ! $? -eq 0 ];then
	logger -t $0 -p local7.info  "flash mount error first time "
    echo "flash mount error first time "
	InitFlashForce
    sleep 0.1
    mount -t ubifs ubifileNum ${mountPath}
	if [ ! $? -eq 0 ];then
		logger -t $0 -p local7.info  "flash mount error"
        echo "flash mount error"
        killall log_process.sh
		SaveLogInner
		exit 1
	fi
fi

logger -t $0 -p local7.info  "flash mount success"
echo "flash mount success"