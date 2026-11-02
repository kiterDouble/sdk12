#! /bin/sh

logPath=$1
logsNum=$2

if [ -z "$1" -o -z "$2" ];then
	echo "sh log_process.sh <日志路径> <日志个数(存储空间大于1G时默认存储7天)>"
	exit 1
fi

logstorePath=${logPath}/logstore/
mkdir -p ${logstorePath}
dirSize=`df -m "${logstorePath}" | awk 'NR==2 {print $2}'`
dirSizeMb=${dirSize}

TarLogFileDate()
{
	cd ${logstorePath}
	find ${logstorePath} -mtime +8 | xargs rm -rf
	dirFile=`ls ${logPath}/ | grep sirunlog. `
	for file in $dirFile
	do
		fileName=`stat -c %z ${logPath}/$file`
		echo ${fileName:0:10}-${fileName:11:8}
		mv ${logPath}/${file} ${logstorePath}/${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.log
		tar zcvf ${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.tgz ${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.log
		rm ${logstorePath}/${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.log
	done
}

TarLogFileTimes()
{
	cd ${logstorePath}
	CurrentNum=`find . -maxdepth 1 -type f | wc -l`
	if [ ${CurrentNum} -gt ${logsNum} ]; then
		find . -maxdepth 1 -type f |ls -lrt |awk -v n=2 'NR <= n {print $9}' | xargs rm -rf
	fi
	dirFile=`ls ${logPath}/ | grep sirunlog. `
	for file in $dirFile
	do
		fileName=`stat -c %z ${logPath}/$file`
		echo ${fileName:0:10}-${fileName:11:8}
		mv ${logPath}/${file} ${logstorePath}/${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.log
		tar zcvf ${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.tgz ${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.log
		rm ${logstorePath}/${fileName:0:10}-${fileName:11:2}_${fileName:14:2}_${fileName:17:2}.log
	done
}



while [ 1 ]
do
	if [ ${dirSizeMb} -gt 1000 ]; then
		TarLogFileDate
	else
		TarLogFileTimes
	fi
    sleep 30
done
