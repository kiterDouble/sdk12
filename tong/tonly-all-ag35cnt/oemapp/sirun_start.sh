#!/bin/sh
echo "sirun start ***********"
#/etc/init.d/start_ql_time_daemon start &
# 0 set net route
echo 1 > /proc/sys/net/ipv4/ip_forward

#�ر�TCP timestam
sysctl -w net.ipv4.tcp_timestamps=0

# 1 set eviroment varialbles
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oemapp/lib
export PATH=$PATH:/oemapp/bin

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

# cp /oemapp/config/log.conf /oemdata/log/
# sh /oemapp/scripts/log_process.sh /oemdata/log/ 30 &

# 2 start init emmc
sh /oemapp/scripts/init_emmc.sh /mnt/sdcard

mountRet=`mount |grep /mnt/sdcard`
if [ $? -eq 0 ]; then
	mkdir -p /mnt/sdcard/log
	mv /oemdata/log/* /mnt/sdcard/log/
	rm -rf /oemdata/log
	ln -s /mnt/sdcard/log /oemdata/log
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

# 清空文件
> /oemdata/export_path.sh
echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:/oemapp/lib/:/oemdata/lib/" >> /oemdata/export_path.sh
echo "export PATH=\$PATH:/oemapp/bin" >> /oemdata/export_path.sh
chmod +x /oemdata/export_path.sh

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
if [ ! -d /sdcard/usbFotaUpgrade/download ];then
    mkdir -p /sdcard/usbFotaUpgrade/download
fi

if [ ! -d /oemdata/config/log ];then
    mkdir -p /oemdata/config/log
fi

if [ ! -d /oemdata/tsp/backup ];then
    mkdir -p /oemdata/tsp/backup
fi

if [ ! -d /sdcard/selfUpgrade ];then
    mkdir -p /sdcard/selfUpgrade
fi

if [ ! -d /sdcard/selfUpgrade/decompress ]; then
    mkdir -p /sdcard/selfUpgrade/decompress
fi

if [ ! -d /sdcard/ota_master/selfUpgrade ]; then
    mkdir -p /sdcard/ota_master/selfUpgrade
fi

if [ ! -d /sdcard/ota_master/selfUpgrade/backup ]; then
    mkdir -p /sdcard/ota_master/selfUpgrade/backup
fi

if [ ! -d /sdcard/ota_master/config ]; then
    mkdir -p /sdcard/ota_master/config
fi


# 复制OTA配置文件到运行时目录
# 如果目标文件不存在或为空，则从/oemapp/config/复制
if [ -f /oemapp/config/support_ota_ecu_config.json ]; then
    if [ ! -f /sdcard/ota_master/config/support_ota_ecu_config.json ] || [ ! -s /sdcard/ota_master/config/support_ota_ecu_config.json ]; then
        cp /oemapp/config/support_ota_ecu_config.json /sdcard/ota_master/config/
    fi
fi
if [ -f /oemapp/config/ota_preconditions_config.json ]; then
    if [ ! -f /sdcard/ota_master/config/ota_preconditions_config.json ] || [ ! -s /sdcard/ota_master/config/ota_preconditions_config.json ]; then
        cp /oemapp/config/ota_preconditions_config.json /sdcard/ota_master/config/
    fi
fi

if [ ! -d /oemdata/szitrus/cert ];then
    mkdir -p /oemdata/szitrus/cert
fi

if [ ! -d /mnt/sdcard/cert ];then
    mkdir -p /mnt/sdcard/cert
fi


if [ ! -d /mnt/sdcard/cfg/pki ];then
    mkdir -p /mnt/sdcard/cfg/pki
fi

if [ ! -d /oemdata/log/pki ];then
    mkdir -p /oemdata/log/pki
fi

if [ ! -d /sdcard/tsp/reissueData ];then
    mkdir -p /sdcard/tsp/reissueData
fi

# 从数据库读取VIN和ICCID写入文件
if [ -f /oemdata/parameters/paramsDb ]; then
    vin_val=$(sqlite3 /oemdata/parameters/paramsDb "select value from paramstbl where id=4;")
    iccid_val=$(sqlite3 /oemdata/parameters/paramsDb "select value from paramstbl where id=0;")
    if [ -n "$vin_val" ]; then
        echo "$vin_val" > /oemdata/tmp/vin.txt
    fi
    if [ -n "$iccid_val" ]; then
        echo "$iccid_val" > /oemdata/tmp/iccid.txt
    fi
    sync
fi

# gpio118设置为低电平
# sh /oemapp/sr_config/gpioCtrl.sh 118 out 0
# # gpio8设置为低电平
# sh /oemapp/sr_config/gpioCtrl.sh 8 out 1

# install spi
insmod /lib/modules/5.4.195/spidev.ko busnum=1 chipselect=0 spimode=1 maxspeed=19200000

iptables -t filter -F
iptables -t nat -F
iptables -t nat -A POSTROUTING -o ccinet1 -j MASQUERADE --random
iptables -t nat -A POSTROUTING -o ccinet2 -j MASQUERADE --random
iptables -t nat -A POSTROUTING -o ccinet3 -j MASQUERADE --random

cp /oemapp/config/log.conf /oemdata/log/
#sh /oemapp/scripts/log_process.sh /oemdata/log 60 &

sleep 2

# 配置gnss门限
#sh /oemapp/scripts/set_gnss_threshold.sh

/oemapp/servers/ag35cet_upgrade_sync
sh /oemapp/scripts/servers_init.sh
#sh /oemapp/scripts/avnt_init.sh
#sh /oemapp/scripts/eth_init.sh

# 如果使用外置es8311 codec,需要打开下面的操作,手动绑一下codec,开机阶段外置codec供电尚未就绪
# echo 0-0018 > /sys/bus/i2c/drivers/es8311-codec/bind

sh /oemapp/scripts/apps_init.sh
#sh /oemapp/scripts/vusapp.sh
sleep 2
sh /oemapp/scripts/udplive.sh &

