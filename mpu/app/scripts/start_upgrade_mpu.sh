#!/bin/sh

if [ -z "$1" -o -z "$2" ];then
    echo "uesage :"
    echo "sh start_upgrade_mpu.sh <升级包路径> <日志路径> "
    echo "ret 0:刷写成功 1:参数个数不正确 2:不满足升级条件 3:指定升级路径不成功 4:指定打印时间不成功 5:执行升级指令不成功 6:升级失败 7:升级状态错乱"
    exit 1
fi

upgradePath=$1
logPath=$2

status=`fotainfo --get-stat | awk -F "[()]+" '{print $2}'`
if [ $status -eq 0 ];then
    date | tee -ai ${logPath}
    echo "upgrade status is ${status}" | tee -a ${logPath}
elif [ $status -eq 3 ];then
    date | tee -ai ${logPath}
    echo "upgrade status is ${status}" | tee -a ${logPath}
else
    date | tee -ai ${logPath}
    echo "upgrade status is ${status}" | tee -a ${logPath}
    exit 2
fi

date | tee -a ${logPath}

fotainfo -p ${upgradePath} | tee -a ${logPath}
if [ $? -ne 0 ];then
    echo "fotainfo -p ${upgradePath} ret :$? " | tee -ai ${logPath}
    exit 3
fi

fotainfo -t 1 | tee -ai ${logPath}
if [ $? -ne 0 ];then
    echo "fotainfo -t 1 ret :$? " | tee -ai ${logPath}
    exit 4
fi

ab_full_update "start-full-fota"  2>&1 | tee -ai ${logPath} 
if [ $? -ne 0 ];then
    echo "ab_full_update "start-full-fota" ret :$? " | tee -ai ${logPath}
    exit 5
fi

while [ 1 ]
do
    status=`fotainfo --get-stat | awk -F "[()]+" '{print $2}'`
    if [ $status -eq 2 ];then
	date | tee -ai ${logPath}
	echo "upgrade status is ${status}" | tee -ai ${logPath}
	exit 0
    elif [ $status -eq 3 ];then
        date | tee -a ${logPath}
        echo "upgrade status is ${status}" | tee -ai ${logPath}
	exit 6
    else
        date | tee -ai ${logPath}
        echo "upgrade status is ${status}" | tee -a ${logPath}
        exit 7
    fi
    sleep 1
done
