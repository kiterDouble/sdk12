#!/bin/bash

ethName=$1
getewayIp=$2

# 清除现有规则
iptables -t mangle -F
iptables -t mangle -X
ip rule del fwmark 100 2>/dev/null
ip rule del fwmark 200 2>/dev/null
ip rule del fwmark 300 2>/dev/null
ip rule del fwmark 400 2>/dev/null
ip rule del fwmark 500 2>/dev/null
ip rule del fwmark 600 2>/dev/null
tc qdisc del dev $ethName root 2>/dev/null

iptables -t mangle -I OUTPUT 1 -d 192.168.1.0/24 -j ACCEPT
iptables -t mangle -I OUTPUT 1 -d 192.168.201.0/24 -j ACCEPT
iptables -t mangle -I OUTPUT 1 -d 192.168.202.0/24 -j ACCEPT
iptables -t mangle -I OUTPUT 1 -d 192.168.205.0/24 -j ACCEPT
iptables -t mangle -I OUTPUT 1 -d 192.168.225.0/24 -j ACCEPT

tc qdisc add dev $ethName root handle 1: prio
tc filter add dev $ethName parent 1: protocol arp u32 match u8 0 0 action skbedit mark 100

# 高级标记 (MARK 100)
iptables -t mangle -A OUTPUT -p icmp -j MARK --set-mark 100
#iptables -t mangle -A OUTPUT -p arp -j MARK --set-mark 100              # ARP协议
iptables -t mangle -A OUTPUT -p tcp --dport 23456 -j MARK --set-mark 100  # TCP:23456
iptables -t mangle -A OUTPUT -p tcp --dport 7999 -j MARK --set-mark 100  # TCP:7999
iptables -t mangle -A OUTPUT -p tcp --dport 25555 -j MARK --set-mark 100  # TCP:25555
iptables -t mangle -A OUTPUT -p udp --dport 53 -j MARK --set-mark 100  # UDP:53
iptables -t mangle -A OUTPUT -p udp --dport 123 -j MARK --set-mark 100  # UDP:123

# 中级标记 (MARK 200)
# iptables -t mangle -A OUTPUT -p icmp -j MARK --set-mark 200             # ICMP协议
iptables -t mangle -A OUTPUT -p tcp --dport 1993 -j MARK --set-mark 200   # TCP:1993
iptables -t mangle -A OUTPUT -p tcp --dport 6993 -j MARK --set-mark 200   # TCP:6993
iptables -t mangle -A OUTPUT -p tcp --dport 7993 -j MARK --set-mark 200   # TCP:7993
iptables -t mangle -A OUTPUT -p tcp --dport 8993 -j MARK --set-mark 200   # TCP:8993
iptables -t mangle -A OUTPUT -p udp --dport 9090 -j MARK --set-mark 200   # UDP:9090


iptables -t mangle -A OUTPUT -p tcp --dport 20000 -j MARK --set-mark 300   # TCP:20000
iptables -t mangle -A OUTPUT -p tcp --dport 1770 -j MARK --set-mark 300   # TCP:1770
iptables -t mangle -A OUTPUT -p tcp --dport 1780 -j MARK --set-mark 300   # TCP:1780
iptables -t mangle -A OUTPUT -p tcp --dport 1790 -j MARK --set-mark 300   # TCP:1790
iptables -t mangle -A OUTPUT -p tcp --dport 1800 -j MARK --set-mark 300   # TCP:1800
iptables -t mangle -A OUTPUT -p tcp --dport 1810 -j MARK --set-mark 300   # TCP:1810
iptables -t mangle -A OUTPUT -p tcp --dport 1820 -j MARK --set-mark 300   # TCP:1820
iptables -t mangle -A OUTPUT -p udp --dport 8000 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8000
iptables -t mangle -A OUTPUT -p udp --dport 8001 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8001
iptables -t mangle -A OUTPUT -p udp --dport 8002 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8002
iptables -t mangle -A OUTPUT -p udp --dport 8003 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8003
iptables -t mangle -A OUTPUT -p udp --dport 8004 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8004
iptables -t mangle -A OUTPUT -p udp --dport 8005 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8005
iptables -t mangle -A OUTPUT -p udp --dport 8200 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8200
iptables -t mangle -A OUTPUT -p udp --dport 8201 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8201
iptables -t mangle -A OUTPUT -p udp --dport 8202 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8202
iptables -t mangle -A OUTPUT -p udp --dport 8203 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8203
iptables -t mangle -A OUTPUT -p udp --dport 8204 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8204
iptables -t mangle -A OUTPUT -p udp --dport 8205 -m dscp ! --dscp 0 -j MARK --set-mark 300  # UDP:8205

iptables -t mangle -A OUTPUT -p udp --dport 8000 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8000
iptables -t mangle -A OUTPUT -p udp --dport 8001 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8001
iptables -t mangle -A OUTPUT -p udp --dport 8002 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8002
iptables -t mangle -A OUTPUT -p udp --dport 8003 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8003
iptables -t mangle -A OUTPUT -p udp --dport 8004 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8004
iptables -t mangle -A OUTPUT -p udp --dport 8005 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8005
iptables -t mangle -A OUTPUT -p udp --dport 8200 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8200
iptables -t mangle -A OUTPUT -p udp --dport 8201 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8201
iptables -t mangle -A OUTPUT -p udp --dport 8202 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8202
iptables -t mangle -A OUTPUT -p udp --dport 8203 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8203
iptables -t mangle -A OUTPUT -p udp --dport 8204 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8204
iptables -t mangle -A OUTPUT -p udp --dport 8205 -m dscp --dscp 0 -j MARK --set-mark 400  # UDP:8205

iptables -t mangle -A OUTPUT -p tcp --dport 9788 -j MARK --set-mark 500   # TCP:9788

# 低级标记 (MARK 300) - 其他所有流量
iptables -t mangle -A OUTPUT -m mark ! --mark 0x0/0x0 -j MARK --set-mark 600

# 策略路由（基于MARK值）
ip rule add fwmark 100 table 100   # 高级路由表
ip rule add fwmark 200 table 200   # 中级路由表
ip rule add fwmark 300 table 300   # 低级路由表
ip rule add fwmark 400 table 400   # 低级路由表
ip rule add fwmark 500 table 500   # 低级路由表
ip rule add fwmark 600 table 600   # 低级路由表

# 创建路由表（示例配置）
ip route add default via $getewayIp dev $ethName table 100  # 高级：专用网关
ip route add default via $getewayIp dev $ethName table 200  # 中级：默认网关+限速
ip route add default via $getewayIp dev $ethName table 300  # 低级：默认网关
ip route add default via $getewayIp dev $ethName table 400  # 高级：专用网关
ip route add default via $getewayIp dev $ethName table 500  # 中级：默认网关+限速
ip route add default via $getewayIp dev $ethName table 600  # 低级：默认网关

#tc qdisc add dev $ethName root handle 1: htb


# 高级流量 (30%带宽)
# tc class add dev $ethName parent 1: classid 1:100 htb rate 30mbit ceil 30mbit

# 中级流量 (50%带宽)
# tc class add dev $ethName parent 1: classid 1:200 htb rate 50mbit ceil 50mbit

# 低级流量 (20%带宽)
# tc class add dev $ethName parent 1: classid 1:300 htb rate 20mbit ceil 20mbit

# 将MARK映射到QoS类
tc filter add dev $ethName protocol ip parent 1:0 handle 100 fw flowid 1:100
tc filter add dev $ethName protocol ip parent 1:0 handle 200 fw flowid 1:200
tc filter add dev $ethName protocol ip parent 1:0 handle 300 fw flowid 1:300
tc filter add dev $ethName protocol ip parent 1:0 handle 400 fw flowid 1:400
tc filter add dev $ethName protocol ip parent 1:0 handle 500 fw flowid 1:500
tc filter add dev $ethName protocol ip parent 1:0 handle 600 fw flowid 1:600


# 启用日志（可选）
iptables -t mangle -A OUTPUT -m mark --mark 100 -j LOG --log-prefix "[HIGH] "
iptables -t mangle -A OUTPUT -m mark --mark 200 -j LOG --log-prefix "[MED] "
iptables -t mangle -A OUTPUT -m mark --mark 300 -j LOG --log-prefix "[LOW] "
iptables -t mangle -A OUTPUT -m mark --mark 400 -j LOG --log-prefix "[LOWEST] "
iptables -t mangle -A OUTPUT -m mark --mark 500 -j LOG --log-prefix "[SPECIAL] "
iptables -t mangle -A OUTPUT -m mark --mark 600 -j LOG --log-prefix "[DEFAULT] "