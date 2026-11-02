#!/bin/sh 

ExportGpio()
{
	repeatTimes=3
	while [ $repeatTimes -ge 1 ]
	do
		echo $1 > /sys/class/gpio/export	
		
		if [ ! -d /sys/class/gpio/gpio$1 ];then
			logger -t $0 -p local7.info "gpio$1 no exist"
			continue
		else
			logger -t $0 -p local7.info "gpio$1 exist" 
			break
		fi
		repeatTimes=$(($repeatTimes-1))
	done
}

SetGpioOut()
{
	repeatTimes=3
	while [  $repeatTimes -ge 1 ]
	do
		echo out > /sys/class/gpio/gpio$1/direction	
		direction=`cat /sys/class/gpio/gpio$1/direction` 
		if [ !  "${direction}" == "out" ];then
			logger -t $0 -p local7.info "direction !="
			continue
		else
			logger -t $0 -p local7.info "direction ==" 
			break
		fi
		repeatTimes=$(($repeatTimes-1))
	done
}

SetGpioValue()
{
	repeatTimes=3
	while [ $repeatTimes -ge 1 ]
	do
		echo $2 > /sys/class/gpio/gpio$1/value
		value=`cat /sys/class/gpio/gpio$1/value` 
		if [ !  ${value} -eq $2 ];then
			logger -t $0 -p local7.info "value != $2 "
			continue
		else
			logger -t $0 -p local7.info "value = $2" 
			break
		fi
		repeatTimes=$(($repeatTimes-1))
	done
}

if [ ! -d /sys/class/gpio/gpio$1 ];then
	ExportGpio $1
#else
	logger -t $0 -p local7.info "/sys/class/gpio/gpio$1 exist"
	#ls /sys/class/gpio/gpio$1
fi


if [ ! -f /sys/class/gpio/gpio$1/direction ];then
	logger -t $0 -p local7.info "/sys/class/gpio/gpio$1/direction is not exist"
	cat /sys/class/gpio/gpio$1/direction
else
	SetGpioOut $1 
fi

if [ ! -f /sys/class/gpio/gpio$1/value ];then
	logger -t $0 -p local7.info "/sys/class/gpio/gpio$1/value is not exist"
	cat /sys/class/gpio/gpio$1/value
else
	SetGpioValue $1 $2
fi
	

