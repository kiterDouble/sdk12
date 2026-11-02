#!/bin/sh
echo "sirun start ***********"
#/etc/init.d/start_ql_time_daemon start &
# 0 set net route
echo 1 > /proc/sys/net/ipv4/ip_forward

#关闭TCP timestam
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
sh /oemapp/scripts/init_emmc.sh /mnt/emmc

mountRet=`mount |grep /mnt/emmc`
if [ $? -eq 0 ]; then
	mkdir -p /mnt/emmc/log
	mv /oemdata/log/* /mnt/emmc/log/
	rm -rf /oemdata/log
	ln -s /mnt/emmc/log /oemdata/log
else
	if [ -L /oemdata/log ]; then
		rm /oemdata/log
	fi
	mkdir -p /oemdata/log
fi

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

iptables -t filter -F
iptables -t nat -F
iptables -t nat -A POSTROUTING -o ccinet1 -j MASQUERADE --random
iptables -t nat -A POSTROUTING -o ccinet2 -j MASQUERADE --random
iptables -t nat -A POSTROUTING -o ccinet3 -j MASQUERADE --random

cp /oemapp/config/log.conf /oemdata/log/
sh /oemapp/scripts/log_process.sh /oemdata/log/ 30 &

/oemapp/servers/ag35cet_upgrade_sync
/oemapp/scripts/servers_init.sh
# 6 start phy 
#echo 10 > /sys/class/gpio/export 
#echo out > /sys/class/gpio/gpio10/direction 
#echo 1 > /sys/class/gpio/gpio10/value
#/etc/init.d/start_emac_le start

# 6 start gps
#echo 24 > /sys/class/gpio/export 
#echo out > /sys/class/gpio/gpio24/direction 
#echo 1 > /sys/class/gpio/gpio24/value

# 7 backup parameters
#sh /oemapp/backup_param.sh

# 8 set net only let
#echo -e 'at+qcfg="nwscanmode",3\r\n' > /dev/smd8

# 9 start time sync 
#sh /oemapp/time/timing.sh &

# sleep 7 移动到了tbox_servere_init.sh
# 10 start server 
#sh /oemapp/servers/tbox_server_init.sh 

# 11 start app
#sleep 0.1
#sh /oemapp/app/app_start.sh 
#sh /oemdata/test.sh &
#sleep 0.1
#start-stop-daemon -S -b -a /oemapp/bin/otamaster &
#sh /oemapp/bin/da_monitor.sh &
#sh /oemapp/bin/start_otamaster.sh &

# 12 start sirun https ota
#mkdir -p /oemdata/fota 
#start-stop-daemon -S -b -a /oemapp/upgrade/fota_tsp &

# 13 start sirun ota
#sleep 2
#sh /oemapp/sirun/udplive.sh &


