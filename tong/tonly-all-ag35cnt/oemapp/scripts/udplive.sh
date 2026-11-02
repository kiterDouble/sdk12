####################################################
#file Name: udplive.sh for GAC T08Y1
# Author: bcs
# Updated Time: 2025-10-15
#########################################################################
#!/bin/sh

num=1
roger="udptimeout"
pkg_dir=/sdcard/47upgrade/download
mkdir -p /oemdata/log/errorlog
LogFile="/oemdata/log/errorlog/log"`date -I`

errorNum=0
applist=""
serverlist=""
iccid=""
vin=""

status_log(){
	cd /oemdata/log/
	if [ $errorNum -le 20 ]; then
		#tar zcvf $LogFile$errorNum".tgz" sirunlog
		du -h /oemapp >> $LogFile
		ls -lh /oemdata/parameters/* >> $LogFile
		top -b -n 2 >> $LogFile
		ifconfig >> $LogFile
		netstat | grep tcp >> $LogFile
		sync
	fi
	
	if [ "$roger" == "log" ] && [ $errorNum -le 2 ]; then
		cd /oemdata/log/errorlog
		#echo "crashed log is uploading..."
		tar_log=${vin:11}_$(date "+%m%d%H%M").tgz
		tar zcvf $tar_log $LogFile $LogFile$errorNum".tgz" /oemdata/log/bootup.txt
		sleep 10
		ftpput -u gitlab-runner -p gitlab-runner 47.97.113.255 $tar_log
		sleep 5
		sendstr="5#"$vin"#"$iccid"#"$version"#"$tar_log
		timeout 6 /oemapp/apps/imok $server_ip $sendstr
		rm -rf $tar_log
		roger="udptimeout"
		sync
	fi
}

doGet(){
	echo "OTA is going on"
	key=${roger#*;}
	key=${key:0:6}
	runcmd=${roger##*;}
	echo $runcmd
	
	# set database value by id
	if [ ${key:2:2} == "id" ]; then
		sid=${key:4}
		setvalue=$runcmd
		sqlite3 /oemdata/parameters/paramsDb "update paramstbl set value='$setvalue' where id=$sid;"
		sleep 2
		sh  /oemapp/vin_is_write.sh
		sync
		sendstr="4#"$vin"#"$iccid"#"$version"#updatedone:"$runcmd
		timeout 16 /oemapp/apps/imok $server_ip $sendstr
		sleep 3
		if [ ${key:0:2} == "re" ]; then
			reboot
		fi
	elif [ $key = "runcmd" ]; then
		sendstr="6#"$vin"#"$iccid"#"$version"#"${runcmd// /_}
		timeout 16 /oemapp/apps/imok $server_ip $sendstr
		`$runcmd`	
	else
		if [ ! -d $pkg_dir ]; then
			mkdir -p $pkg_dir
		fi
		cd $pkg_dir
		rm -rf $pkg_dir/*
		echo $key > $pkg_dir/key	
		eval "$runcmd"
		logger -t $0 -p local7.info "执行FTP命令:$runcmd"
		sleep 70
	fi
}

getParams(){ 
	while true; do	
		iccid=`cat /sdcard/tmp/iccid.txt`
		vin=`cat /sdcard/tmp/vin.txt`
		# 从数据库获取vin 和 iccid
		iccid=$(sqlite3 /oemdata/parameters/paramsDb "select value from paramstbl where id=0;")
		echo $iccid > /sdcard/tmp/iccid.txt
		sync
		vin=$(sqlite3 /oemdata/parameters/paramsDb "select value from paramstbl where id=4;")
		echo "$vin" > /sdcard/tmp/vin.txt
		sync
		# 参数合法退出、不合法则重新获取
		if  [ "$iccid" != "00000000000000000000" ] && [ "$vin" != "00000000000000000" ]; then
			echo "VIN has been obtained: $vin"
			echo "ICCID has been obtained: $iccid"
			break
		else
			echo "Params is illegal, waiting .."
			sleep 5
		fi
	done
}

date >> /oemdata/log/bootup.txt
# 删除 5 天前的 .tgz日志文件
# find /oemdata/log/errorlog/ -mtime +5 -name "*.tgz" -exec rm -rf {} \;
# 删除大于 40MB 的文件​
# find /oemdata/data/ -name "*" -type f -size +40000k | xargs -n 1 rm -f
# sync

server_ip="47.97.113.255"
version=`cat /oemapp/version.txt`

if [ ! -f "/sdcard/tmp/iccid.txt" ]; then
    mkdir -p "/sdcard/tmp/" && touch "/sdcard/tmp/iccid.txt"
fi

if [ ! -f "/sdcard/tmp/vin.txt" ]; then
    mkdir -p "/sdcard/tmp/" && touch "/sdcard/tmp/vin.txt"
fi

logger -t $0 -p local7.info "周期获取参数(vin/iccid)中..."
getParams
logger -t $0 -p local7.info "参数 已获取: vin[$vin] iccid[$iccid]"
bootupnum=`cat /oemdata/log/bootup.txt | wc -l`

/oemapp/scripts/check_pkg.sh &

#上报记录的重启次数
sendstart="1#"$vin"#"$iccid"#"$version"#"$bootupnum"#"0
timeout 6 /oemapp/apps/imok $server_ip ${sendstart}

count=0
start_time=$(date +%s)
while true; do
	vin=`cat /sdcard/tmp/vin.txt`
    current_time=$(date +%s)
    elapsed=$((current_time - start_time))
	roger="udptimeout"
    # 前 60 秒内：每 5 秒发一次心跳 0#
    if [ $elapsed -lt 60 ]; then
		sendstr="0#"$vin"#"$version"#"$num
		roger=`timeout 3 /oemapp/apps/imok $server_ip $sendstr` 
		echo "roger:$roger"
		logger -t $0 -p local7.info "心跳包发送$roger"
		num=$((num + 1))
    else
        # 超过 1 分钟后：每1分钟发一次心跳 0#
        if [ $((count % 12)) -eq 0 ]; then
            sendstr="0#"$vin"#"$version"#"$num
			roger=`timeout 3 /oemapp/apps/imok $server_ip $sendstr`
			echo "roger:$roger"
			logger -t $0 -p local7.info "心跳包发送$roger" 
			num=$((num + 1))
        fi

        # 超过 1 分钟后：每1小时发一次心跳 1#
        if [ $((count % 720)) -eq 0 ]; then
            sendstr="1#"$vin"#"$iccid"#"$version"#"$num
			roger=`timeout 3 /oemapp/apps/imok $server_ip $sendstr` 
			echo "roger:$roger"
			logger -t $0 -p local7.info "心跳包发送$roger"
			num=$((num + 1))
        fi
    fi

	isOTA="notOTA"
	if [ "$roger" != "udptimeout" ]; then
		isOTA=${roger%%;*}
		echo $isOTA
		if [ "$isOTA" = "ALL" ] || [ "$isOTA" = "$vin" ]; then
		doGet
		fi
	fi

    sleep 5
    count=$((count + 1))

	# 检查进程是否存在
	if pgrep "imok" >/dev/null; then
		# 进程存在，终止它
		killall imok
	fi
done


