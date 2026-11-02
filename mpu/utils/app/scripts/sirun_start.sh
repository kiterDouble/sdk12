#!/bin/sh
echo "sirun start ***********"
#/etc/init.d/start_ql_time_daemon start &
# 0 set net route
echo 1 > /proc/sys/net/ipv4/ip_forward

#关闭TCP timestam
sysctl -w net.ipv4.tcp_timestamps=0

#cpu资源限制
mkdir -p /sys/fs/cgroup/cpu,cpuacct/limit_cpu
echo 15000 > /sys/fs/cgroup/cpu,cpuacct/limit_cpu/cpu.cfs_quota_us

# 1 set eviroment varialbles
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oemapp/lib/

sleep 5
sh /oemapp/scripts/init_emmc.sh  &

if [ -L /oemdata/log ]; then
    rm /oemdata/log
fi

if [ -L /oemdata/upgrade ]; then
    rm /oemdata/upgrade
fi

if [ ! -d /oemdata/log ];then
    mkdir -p /oemdata/log
fi

if [ ! -d /oemdata/tmp ];then
        mkdir  /oemdata/tmp
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

if [ ! -d /mnt/emmc/token ];then
        mkdir  /mnt/emmc/token
fi

if [ ! -d /oemdata/token ];then
        mkdir  /oemdata/token
fi

if [ ! -d /oemdata/cert ];then
        mkdir  /oemdata/cert
fi

if [ ! -d /oemdata/log/can ];then
        if [ ! -d /oemdata/log ];then
		mkdir -p /oemdata/log/can
	else
		mkdir /oemdata/log/can
	fi
fi
date >> /oemdata/log/bootup.txt
iptables -t filter -F
iptables -t nat -F
iptables -t nat -A POSTROUTING -o rmnet_data0 -j MASQUERADE --random

cp /oemapp/config/mcu.ini /oemdata/

cp /oemapp/config/log.conf /oemdata/log/
/oemapp/scripts/log_process.sh /oemdata/log/ 30 & echo $! > /sys/fs/cgroup/cpu,cpuacct/limit_cpu/tasks

mkdir -p /var/run/sshd
chmod  644 /var/run/sshd
systemctl  restart sshd.service

systemctl stop chronyd.service 
/usr/sbin/chronyd -f /oemapp/config/chrony.conf

ql_cmd  at at+qcfg=\"ims\",2
if [ -b /dev/mmcblk0 ];then
        ql_cmd  at at+qnwprefcfg=\"ue_usage_setting\",1
fi
/oemapp/servers/ag591_upgrade_sync 

/oemapp/scripts/init_net_vlan.sh

echo "server start..."
/oemapp/scripts/servers_init.sh
echo "server end..."

if [ -b /dev/mmcblk0 ];then
    mkdir -p /oemdata/tts/
    cp /oemapp/audio/* -rf /oemdata/tts/
fi
sleep 2

echo "apps start..."
/oemapp/scripts/apps_init.sh 
echo "apps end..."

# 修改DB文件手段
# /oemapp/scripts/pbdata_fix.sh
sleep 1
/oemapp/scripts/sqlite3_update.sh
sleep 1

echo "carota app start..."
/oemapp/scripts/carota_app_start.sh &
echo "carota app end..."

sleep 3

echo "fcwl app start..."
# /oemapp/scripts/fcwl_init.sh &
echo "fcwl app end..."

# 删除coredump文件
if [ -f "/data/coredump/core.audio_app" ]; then
    rm -f "/data/coredump/core.audio_app"
fi
