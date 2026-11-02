#!/bin/bash

# 设置运行时间（10小时 = 36000秒）
RUNTIME=$((2 * 60 * 60))
START_TIME=$(date +%s)
logpathname=/oemdata/log/dns_err.log
# 主循环
while true; do
    CURRENT_TIME=$(date +%s)
    ELAPSED_TIME=$((CURRENT_TIME - START_TIME))
    
	if [ ! -f "/tmp/ql_lan_resolv.conf" ]; then
		echo "文件 /tmp/ql_lan_resolv.conf 不存在，跳过本次循环" >> ${logpathname}
		sleep 60
		START_TIME=$(date +%s)
		continue
	fi
	
	echo "已运行（$ELAPSED_TIME) 秒 " >> ${logpathname}
	
    # 检查是否达到运行时间
    if [ $ELAPSED_TIME -ge $RUNTIME ]; then
        echo "===============================================" >> ${logpathname}
        echo "已运行（$ELAPSED_TIME 秒），开始执行后续命令" >> ${logpathname}
        echo "当前时间: $(date '+%Y-%m-%d %H:%M:%S')" >> ${logpathname}
		rm /tmp/ql_lan_resolv.conf 
		echo "删除指令结果: $?" >> ${logpathname}
		ql_cmd nf stop dns_srv dns_lan
		echo "停止服务结果: $?" >> ${logpathname}
		sleep 1
		ql_cmd nf start  dns_srv dns_lan
		echo "启动服务结果: $?" >> ${logpathname}
		cat /etc/resolv.conf  > /tmp/ql_lan_resolv.conf
		echo "设置DNS服务器: $?" >> ${logpathname}
        echo "重启服务时间：" `date`   >> ${logpathname}
		START_TIME=$(date +%s)
    fi
    
	nslookup  www.baidu.com 127.0.0.1
    if [ $? -ne 0 ]; then
		echo "nslookup error" `date`    >> ${logpathname}
		rm /tmp/ql_lan_resolv.conf
		echo "删除指令结果: $?"  >> ${logpathname}
		ql_cmd nf stop dns_srv dns_lan
		echo "停止服务结果: $?"  >> ${logpathname}
		sleep 1
		ql_cmd nf start  dns_srv dns_lan
		echo "启动服务结果: $?"  >> ${logpathname}
		cat /etc/resolv.conf  > /tmp/ql_lan_resolv.conf
		echo "设置DNS服务器: $?" >> ${logpathname}
		echo "重启服务时间：" `date`   >> ${logpathname}
		START_TIME=$(date +%s)
	fi
	sleep 30
done
