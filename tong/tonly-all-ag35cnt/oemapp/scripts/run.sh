####################################################
#file Name: run.sh for GAC T08Y1
# Author: bcs
# Updated Time: 2025-10-15
#########################################################################
#!/bin/sh

# kill check_pkg.sh
check_id=`ps | grep "check_pkg" | grep -v "grep" | awk '{print $1}'`
check_id=${check_id:0:5}
if [ -n "$check_id" ];then
	echo $check_id
	kill -9 $check_id
else
	echo "no check_pkg"
fi	

vin=`cat /sdcard/tmp/vin.txt`
iccid=`cat /sdcard/tmp/iccid.txt`
version="209"
if [ -s /oemapp/version.txt ]; then
	version=`cat /oemapp/version.txt`
fi
server_ip="47.97.113.255"
Bport="209" 


version_id=${version:0:3}

if [ $version_id == $Bport ];then


chmod +x /sdcard/47upgrade/download/*

pkg_dir=/sdcard/47upgrade/

rm -rf /sdcard/47upgrade/*.bin
rm -rf /sdcard/47upgrade/mcu*
rm -rf /sdcard/47upgrade/*update*.zip

sync

date >> /sdcard/47upgrade/upgradetime.txt

cpukey="100418"
mcukey="b6fc4c"

cd $pkg_dir

cpukey="fc223d"
mcukey="e3d4d8"
cpukey="b4c93e"
mcukey="da304e"
timeout 6 /oemapp/apps/imok $server_ip $sendstr
logger -t $0 -p local7.info "cpu下载完成,imok返回值:$?"
mv /sdcard/47upgrade/download/*update*.zip /sdcard/47upgrade/update.zip
sync
sleep 2

mv /sdcard/47upgrade/download/mcu*.tgz /sdcard/47upgrade/mcu.tgz
tar -xzvf /sdcard/47upgrade/mcu.tgz -C /sdcard/47upgrade/
cp -rf /sdcard/47upgrade/* /sdcard/selfUpgrade/decompress/
sync
sleep 2

end_function()
{
	cp -f /var/log/messages /oemdata/log/
	sync
	sleep 3
}
# 校验key
failnum=0
while [ $failnum -le 5 ]; do

	loc_md5=`md5sum mcu.tgz`
	key1=${loc_md5:9:3}${loc_md5:5:2}${loc_md5:2:1}
	loc_md5=`md5sum update.zip`
	key2=${loc_md5:9:3}${loc_md5:5:2}${loc_md5:2:1}
	if [ $mcukey = $key1 ]&& [ $cpukey = $key2 ]; then
		sendstr="3#"$vin"#"$iccid"#"$version"#key校验通过"
		timeout 6 /oemapp/apps/imok $server_ip $sendstr
		logger -t $0 -p local7.info "key校验通过,imok返回值:$?"
		echo "check key success"
		failnum=99

	else
		failnum=`expr $failnum + 1`
		echo $failnum
		if [ $failnum -ge 4 ]; then
			rm -rf $pkg_dir/*.zip
			rm -rf $pkg_dir/*.tgz
			sendstr="3#"$vin"#"$iccid"#"$version"#key校验失败"$key1,$key2
			timeout 6 /oemapp/apps/imok $server_ip $sendstr
			logger -t $0 -p local7.info "key校验失败,imok返回值:$?"
			echo "check key failed"
			rm -rf /sdcard/47upgrade/download/*
			sh /oemapp/scripts/check_pkg.sh &
			end_function
			exit 0
	
	sleep 10
		fi
	fi
done


#upgrade MCU
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/oemapp/lib/
cpu_ret=0

upgrade_function()
{
    mcu_ret=1
    timeout 150 /oemapp/apps/otaDaemon 1
    mcu_ret=$?
    if [ "$mcu_ret" -eq 0 ]; then
        sendstr="3#"$vin"#"$iccid"#"$version"#Upgrade_finish!"
		logger -t $0 -p local7.info "Upgrade_finish,imok返回值:$?"
		timeout 6 /oemapp/apps/imok $server_ip $sendstr
		send_ret = $?
		while [ $send_ret -ne 0 ]; do
			sleep 1
			logger -t $0 -p local7.info "Upgrade_finish,imok返回值:$?"
			timeout 6 /oemapp/apps/imok $server_ip $sendstr
			send_ret = $?
		done
    else
        sendstr="3#"$vin"#"$iccid"#"$version"#Upgrade_Fail!"
		timeout 6 /oemapp/apps/imok $server_ip $sendstr
		logger -t $0 -p local7.info "Upgrade_Fail,imok返回值:$?"
		send_ret = $?
		while [ $send_ret -ne 0 ]; do
			sleep 1
			logger -t $0 -p local7.info "Upgrade_finish,imok返回值:$?"
			timeout 6 /oemapp/apps/imok $server_ip $sendstr
			send_ret = $?
		done
		cpu_ret=1
    fi
}

upgrade_count=0
upgrade_function
echo "++++++++++"
echo $cpu_ret
echo "++++++++++"

rm -rf /sdcard/47upgrade/download/*
rm -rf /sdcard/47upgrade/mcu*
rm -rf /sdcard/47upgrade/*update*.zip
rm -rf /sdcard/47upgrade/run.sh
rm -rf /sdcard/selfUpgrade/decompress/*
sleep 3
sync

sendstr="3#"$vin"#"$iccid"#"$version"#Clear!"
timeout 6 /oemapp/apps/imok $server_ip $sendstr
logger -t $0 -p local7.info "Clear,imok返回值:$?"

else
	rm -rf /sdcard/47upgrade/download/*
	rm -rf /sdcard/47upgrade/mcu*
	rm -rf /sdcard/47upgrade/*update*.zip
	rm -rf /sdcard/47upgrade/run.sh
	sleep 5
	sync
	sendstr="3#"$vin"#"$iccid"#"$version"#项目号无法匹配_upgrade_fail!"
	timeout 6 /oemapp/apps/imok $server_ip $sendstr
	logger -t $0 -p local7.info "项目号无法匹配_upgrade_fail,imok返回值:$?"
    echo "program num is invalid"
fi

end_function

reboot
