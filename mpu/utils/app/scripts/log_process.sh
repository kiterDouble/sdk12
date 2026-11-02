#! /bin/sh

# 定义检查并清理磁盘空间的函数
clean_disk_when_low() {
    # 设置阈值为100MB
	mount_point=$1
	thresholdNum=$2
	findDir=$3
    
    # 获取挂载分区的剩余空间（MB），保留整数部分
    # df -BM 以MB为单位显示，awk提取剩余空间列并去掉M单位
    local free_space=$(df -BM "$mount_point" | awk 'NR==2 {gsub(/M/,""); print $4}')
    # 确保free_space是数字
    if ! [[ "$free_space" =~ ^[0-9]+$ ]]; then
        echo "错误：无法获取有效的剩余空间数值"
        return 1
    fi
    
    echo "当前挂载分区: $mount_point"
    echo "剩余空间: ${free_space}MB"
    echo "阈值: ${thresholdNum}MB"
    
    # 判断剩余空间是否小于阈值
    if [ "$free_space" -lt "$thresholdNum" ]; then
        echo "警告：剩余空间小于${thresholdNum}MB，开始清理..."
        
        # 查找该分区下创建时间最早的普通文件（排除目录、符号链接等）
        # -xdev 限制在当前文件系统，避免跨分区查找
        # -type f 只找普通文件
        # -printf "%C@ %p\n" 打印文件创建时间戳和路径
        # sort -n 按时间戳排序（最早的在前）
        # head -n1 取第一个（最早的）
        # awk '{print $2}' 提取文件路径
        local oldest_file=$(ls -lt --full-time "$findDir"/* 2>/dev/null | grep -v '^d' | sort -k6 | head -n1 | awk '{print $9}')
        
        if [ -n "$oldest_file" ]; then
            echo "将删除创建时间最早的文件: $oldest_file"
            # 先打印文件信息，再删除
            ls -l "$oldest_file"
            rm -f "$oldest_file"
            if [ $? -eq 0 ]; then
                echo "文件删除成功"
            else
                echo "错误：文件删除失败"
                return 1
            fi
        else
            echo "错误：未找到可删除的文件"
            return 1
        fi
    else
        echo "剩余空间充足，无需清理"
        return 0
    fi
}

tar_log_file()
{
	logPath=$1
	cd ${logPath}/logstore/
	dirFile=`ls /oemdata/log/ | grep sirunlog. `
	for file in $dirFile
	do
		fileName=`stat -c %z /oemdata/log/$file`
		echo "${fileName:0:10}-${fileName:11:8}"  >> /oemdata/log/sirunlog
		cp /oemdata/log/${file} ${logPath}/logstore/${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.log
		tar zcvf ${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.tgz ${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.log
		rm ${logPath}/logstore/${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.log
		rm /oemdata/log/${file}
	done
}

check_log_file()
{
    #cd /oemdata/${logPath}/logstore/
	dirpath="${logPath}/logstore/"
	file_count=$(ls -l ${logPath}/logstore/| grep -v '^d' | wc -l)
	diff=$(expr $file_count - $maxLog)
	while [ $diff -gt 1 ]
	do
	# 输出文件数量
	echo "当前文件夹下的文件数量为: $file_count"  >> /oemdata/log/sirunlog
	oldest_file="$dirpath$(ls -ltr1 $dirpath |  head -n 2 | tail -n 1 | awk '{print $NF}')"
	echo "$oldest_file"  >> /oemdata/log/sirunlog
	rm -f $oldest_file
	file_count=$(ls -l ${logPath}/logstore/| grep -v '^d' | wc -l)
	diff=$(expr $file_count - $maxLog)
	echo "当前文件夹下的超出设定数量为: $diff"  >> /oemdata/log/sirunlog
	sleep 1
	done
}

check_space()
{
    space=`df ${logPath} | grep /mnt/flash | awk -F" " '{print $4}'|awk -F. '{print $1}'`
    dirpath="${logPath}/logstore/"
	out_counter=0
	while [ $space -lt 5000 ]
	do
		#logger -t $0 -p local7.info "upgrade space < 5M space:$space"
		echo "upgrade space < 5M space:$space" >> /oemdata/log/sirunlog
		oldest_file="$dirpath$(ls -ltr1 $dirpath | head -n 2 | tail -n 1 | awk '{print $NF}')"
		rm -f $oldest_file
		#logger -t $0 -p local7.info "rm oldest:$oldest_file"
		echo "rm oldest:$oldest_file" >> /oemdata/log/sirunlog
		space=`df ${logPath} | grep /mnt/flash | awk -F" " '{print $4}'|awk -F. '{print $1}'`
		out_counter=$((out_counter + 1))
		if [ $out_counter -gt 100 ];then
			rm -rf ${logPath}/*
			sleep 1
			reboot
		fi
	    usleep 100000
	done
}

check_inner_space()
{
	inner_counter=0
    inner_space=`df /oemdata | grep oemdata | awk -F" " '{print $4}'|awk -F. '{print $1}'`
    inner_dirpath="/oemdata/databak/"
	while [ $inner_space -lt 5000 ]
	do
		#logger -t $0 -p local7.info "inner space < 5M space:$inner_space"
		echo "inner space < 5M space:$inner_space" >> /oemdata/log/sirunlog
		inner_oldest_file="$inner_dirpath$(ls -ltr1 $inner_dirpath | grep "realdata" | head -n 1 | awk '{print $NF}')"
		rm -f $inner_oldest_file
		#logger -t $0 -p local7.info "rm oldest:$inner_oldest_file"
		echo "rm oldest:$inner_oldest_file" >> /oemdata/log/sirunlog
		inner_space=`df /oemdata | grep oemdata | awk -F" " '{print $4}'|awk -F. '{print $1}'`
		inner_counter=$((inner_counter + 1))
		if [ $inner_counter -gt 500 ];then
			rm -rf /oemdata/databak/*
			rm -rf /oemdata/upgrade/*
			rm -rf /oemdata/flash/mdadownload/dm/*
			echo "rm /oemdata/databak/" >> /oemdata/log/sirunlog
			sleep 1
			#reboot 去掉重启，防止循环重启
		fi
		usleep 100000
	done

	srlogfile=`du /oemdata/log/sr.log |awk -F. '{print $1}'`
	if [ $srlogfile -gt 5000 ];then
		rm /oemdata/log/sr.log
		echo "rm /oemdata/log/sr.log" >> /oemdata/log/sirunlog
	fi

	bootupfile=`du /oemdata/log/bootup.txt |awk -F. '{print $1}'`
	if [ $bootupfile -gt 5000 ];then
		rm /oemdata/log/bootup.txt
		echo "rm /oemdata/log/bootup.txt" >> /oemdata/log/sirunlog
	fi

	#dmesgfile=`du /oemdata/log/dmesg.log |awk -F. '{print $1}'`
	#if [ $dmesgfile -gt 5000 ];then
	#	rm /oemdata/log/dmesg.log
	#	echo "rm /oemdata/log/dmesg.log" >> /oemdata/log/sirunlog
	#fi

	#logcatfile=`du /oemdata/log/logcat.log |awk -F. '{print $1}'`
	#if [ $logcatfile -gt 5000 ];then
	#	rm /oemdata/log/logcat.log
	#	echo "rm /oemdata/log/logcat.log" >> /oemdata/log/sirunlog
	#fi
}

#上电2分钟内不做日志压缩，解决时间未同步时压缩文件时间过早问题。
#防止时间一直未同步成功，日志不压缩问题。
sleep 120

while [ 1 ]
do
	mount | grep -q '^/dev/.* /mnt/srlog '
	ret=$?
	if [ $ret -eq 0 ];then
		mkdir -p /mnt/srlog/logstore
		logstoredir=/mnt/srlog/
		thresholdNum=100
		if [ ! -L /mnt/srlog/sirunlog ]; then
        	ln -s /oemdata/log/sirunlog  /mnt/srlog/sirunlog
        fi

		if [ -d /oemdata/log/logstore ]; then
        	mv /oemdata/log/logstore/* /mnt/srlog/logstore/
			rm -r /oemdata/log/logstore
        fi

	else
		mkdir -p /oemdata/log/logstore
		logstoredir=/oemdata/log/
		thresholdNum=130
	fi

	clean_disk_when_low ${logstoredir} ${thresholdNum} ${logstoredir}/logstore

	upgrade_check=`fotainfo --get-stat | awk -F "[()]+" '{print $2}'`
	if [ $upgrade_check -eq 1 ];then
		#logger -t $0 -p local7.info "upgrading,stop tar"
		echo "upgrading,stop tar" >> /oemdata/log/sirunlog
	else
		tar_log_file ${logstoredir}
	fi
	
    sleep 30
done

