#!/bin/sh
echo "sirun start ***********"
#/etc/init.d/start_ql_time_daemon start &
# 0 set net route
echo 1 > /proc/sys/net/ipv4/ip_forward

#¹Ø±ÕTCP timestam
sysctl -w net.ipv4.tcp_timestamps=0

# 1 set eviroment varialbles
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oemapp/lib/

# 2 start init flash
#sh /oemapp/scripts/init_flash.sh /mnt/flash

# mountRet=`mount |grep /mnt/flash`
# if [ $? -eq 0 ]; then
# 	mkdir -p /mnt/flash/log
# 	mv /oemdata/log/* /mnt/flash/log/
# 	rm -rf /oemdata/log
# 	ln -s /mnt/flash/log /oemdata/log
# else
# 	if [ -L /oemdata/log ]; then
# 		rm /oemdata/log
# 	fi
# 	mkdir -p /oemdata/log
# fi

# 2 start init emmc
# sh /oemapp/scripts/init_emmc.sh /mnt/emmc

# mountRet=`mount |grep /mnt/emmc`
# if [ $? -eq 0 ]; then
# 	mkdir -p /mnt/emmc/log
# 	mv /oemdata/log/* /mnt/emmc/log/
# 	rm -rf /oemdata/log
# 	ln -s /mnt/emmc/log /oemdata/log
# else
# 	if [ -L /oemdata/log ]; then
# 		rm /oemdata/log
# 	fi
# 	mkdir -p /oemdata/log
# fi

# 3 start sshd
#mkdir -p  /usrdata/log
#/sbin/sshd -f /etc/sshd_config -E /usrdata/log/ssh.log

# 4 start dbus
#mkdir /run/dbus
#/usr/bin/dbus-daemon  --system

# 5 mkdir needed directroy
if [ ! -d /oemdata/tmp ];then
	mkdir  /oemdata/tmp
fi

if [ ! -d /oemdata/log ];then
	mkdir  /oemdata/log
fi

if [ ! -d /oemdata/parameters ];then
	mkdir /oemdata/parameters
fi

if [ ! -d /oemdata/data ];then
	mkdir /oemdata/data
fi

if [ ! -d /oemdata/databak ];then
	mkdir /oemdata/databak
fi
if [ ! -d /oemdata/upgrade/download ];then
    mkdir -p /oemdata/upgrade/download
fi

if [ ! -d /oemdata/config/log ];then
    mkdir -p /oemdata/config/log
fi

if [ -d /oemdata ];then
    cp /oemapp/data/* /oemdata/
    chmod 777 /oemdata/*
fi

iptables -t filter -F
iptables -t nat -F
iptables -t nat -A POSTROUTING -o ccinet1 -j MASQUERADE --random
iptables -t nat -A POSTROUTING -o ccinet2 -j MASQUERADE --random
iptables -t nat -A POSTROUTING -o ccinet3 -j MASQUERADE --random

cp /oemapp/config/log.conf /oemdata/log/
sh /oemapp/scripts/log_process.sh /oemdata/log/ 30 &

/oemapp/scripts/servers_init.sh

