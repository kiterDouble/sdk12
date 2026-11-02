#!/bin/sh

if [ -z "$1" ];then
    echo "uesage :"
    echo "sh get_upgrade_status.sh <升级包路径> "
    echo "ret 0:成功 1:升级中  2:需备份  3:升级失败  4:备份中  5:指令中缺少升级包路径"
    exit 5
fi

upgradeDir=$1
backupFlag=0

if [ -d ${upgradeDir}/tmpupdate ];then
    backupFlag=1
    echo ${backupFlag}
fi

status=`fotainfo --get-stat | awk -F "[()]+" '{print $2}'`
echo "ret 0:成功 1:升级中  2:需备份  3:升级失败  4:备份中  5:指令中缺少升级包路径"

if [ ${backupFlag} -eq 1 ] && [ ${status} -eq 2 ];then
    echo "upgrade status is 4" | tee -a ${logPath}
    exit 4
else
    echo "upgrade status is ${status}" | tee -a ${logPath}
    exit ${status}
fi
