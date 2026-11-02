#!/bin/bash
LOGFILE="/oemdata/log/sr.log"

mkdir -p /oemdata/fcwl

cp /oemapp/apps/fcwl-mp.conf    /oemdata/fcwl/
cp /oemapp/apps/fcwl-mp-client  /oemdata/fcwl/

brctl delif bridge0 ecm0 
echo host > /sys/devices/platform/soc/a600000.ssusb/mode 
echo 80 > /sys/class/gpio/export 
echo out > /sys/class/gpio/gpio80/direction 
echo 1 > /sys/class/gpio/gpio80/value 
sleep 5
brctl  addif bridge0 usb0 
ifconfig bridge0 192.168.225.10 netmask 255.255.255.0 up 
ifconfig usb0 down  
ifconfig usb0 up

RunApp()
{
    while true
    do
    /oemdata/fcwl/fcwl-mp-client -c /oemdata/fcwl/fcwl-mp.conf 
    # date >> ${LOGFILE}
    # echo "/oemdata/fcwl/fcwl-mp-client  exit." >> ${LOGFILE}
    sleep 1
    done
}

RunApp

