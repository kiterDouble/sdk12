#!/bin/sh

# 授权IP列表
ALLOWED_IPS="
192.168.1.2
192.168.1.3
192.168.1.9
192.168.1.100
192.168.201.2
192.168.201.3
192.168.202.2
192.168.202.3
192.168.1.21
192.168.1.8
192.168.1.20
192.168.1.99
192.168.1.201
192.168.1.202
192.168.201.1
192.168.202.1
192.168.1.1
192.168.1.86
192.168.1.88
192.168.205.10
192.168.205.15
192.168.225.10
192.168.225.1
169.254.4.1
192.168.1.222
"
# ++++++++++++++++++++++++ allowed ip 初始化列表 ++++++++++++++++++++++++ #
allow_ip_init() {
    # 清空INPUT链
    iptables -F INPUT 2>/dev/null
    sleep 0.1

    # 设置默认策略为DROP
    iptables -P INPUT DROP
    sleep 0.1

    # 添加授权IP
    for ip in $ALLOWED_IPS; do
        if [ -n "$ip" ]; then
            iptables -A INPUT -s "$ip" -j ACCEPT
            sleep 0.1
        fi
    done

    # 添加本地回环
    iptables -A INPUT -i lo -j ACCEPT
    sleep 0.1

    # 添加已建立连接
    iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT

    return 0
}

# ++++++++++++++++++++++++ 检查 授权IP 配置 ++++++++++++++++++++++++ #
allow_ip_check_and_repair() {
    # 1. 检查默认策略（这个必须特殊处理）
    local policy=$(iptables -L INPUT -n 2>/dev/null | head -1 | awk '{print $4}' | tr -d '()')
    if [ "$policy" != "DROP" ]; then
        iptables -P INPUT DROP
    fi

    sleep 0.1

    # 2. 检查本地回环规则
    if ! iptables -C INPUT -i lo -j ACCEPT 2>/dev/null; then
        iptables -A INPUT -i lo -j ACCEPT
    fi

    sleep 0.1

    # 3. 检查已建立连接规则
    if ! iptables -C INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT 2>/dev/null; then
        iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
    fi

    sleep 0.1

    # 4. 逐个检查所有授权IP
    for ip in $ALLOWED_IPS; do
        if [ -n "$ip" ]; then
            if ! iptables -C INPUT -s "$ip" -j ACCEPT 2>/dev/null; then
                iptables -A INPUT -s "$ip" -j ACCEPT
                sleep 0.1
            fi
        fi
    done
}


# ++++++++++++++++++++++++ 周期检查 授权IP 配置 ++++++++++++++++++++++++ #
allow_ip_check() {
    while true; do
        allow_ip_check_and_repair > /dev/null 2>&1
        sleep 20
    done
}


# ++++++++++++++++++++++++ 主模组借网功能,自动检查并重新创建路由 ++++++++++++++++++++++++ #
main_module_check_bridge_route() {
    while true; do
        route_exists=$(ip route show default | grep -c "via 192.168.1.202 dev eth0 src 192.168.1.201 metric 200")
        if [ ${route_exists} -eq 0 ]; then
            ip route add default via 192.168.1.202 dev eth0 src 192.168.1.201 metric 200
        fi
        sleep 20
    done
}

# ++++++++++++++++++++++++ 主模组借网功能 ++++++++++++++++++++++++ #
main_module_add_bridge_route() {
    ip route add default via 192.168.1.202 dev eth0 src 192.168.1.201 metric 200
    sleep 0.1
    iptables -t nat -A OUTPUT -s 192.168.1.201 -p udp --dport 53 -j DNAT --to 192.168.1.202:53
    sleep 0.1
    iptables -t nat -A OUTPUT -s 192.168.1.201 -p tcp --dport 53 -j DNAT --to 192.168.1.202:53
    sleep 0.1
}

sleep 3

if [ -b /dev/mmcblk0 ];then
    echo  0 > /sys/class/net/eth0/rx_clock_rdy
    ifconfig eth0 down
    sleep 0.5
    vconfig add eth0 201
    sleep 0.1
    ifconfig eth0.201 192.168.201.1 netmask 255.255.255.0 broadcast 192.168.201.255 up
    sleep 0.1
    vconfig add eth0 205
    sleep 0.1
    ifconfig eth0.205 192.168.205.10 netmask 255.255.255.0 broadcast 192.168.205.255 up
    sleep 0.1
    ifconfig eth0:0 192.168.1.201 netmask 255.255.255.0 up
    sleep 0.1
    # ifconfig eth0:1 192.168.1.8 netmask 255.255.255.0 up
    sleep 0.1
    brctl delif bridge0 eth0
    # ip link add name bridge1 type bridge
    # sleep 0.1
    # ip link add name bridge2 type bridge
    # sleep 0.1
    # ifconfig bridge1 192.168.1.201 netmask 255.255.255.255 broadcast 192.168.1.255 up
    # sleep 0.1
    # ip link set eth0 master bridge2
    # sleep 0.1
    # ifconfig bridge2 192.168.1.88 netmask 255.255.255.0 broadcast 192.168.1.255 up
    # sleep 0.1
    echo  1 > /sys/class/net/eth0/rx_clock_rdy
    ifconfig eth0 up
    sleep 0.1
    # ++++++++++++++++++++++++ 主模组借网功能，开启借网路由指向副模组 192.168.1.202 ++++++++++++++++++++++++ #
    main_module_add_bridge_route
    sleep 0.1
    main_module_check_bridge_route &
    sleep 0.1
    # --------------------------------------------------------------------------------------------------- #
    touch /oemdata/log/udhcpd.leases; udhcpd -f /oemapp/config/udhcpd_bridge2.conf &
    sleep 0.1
    /oemapp/scripts/monitor_dns_1.sh &
else
    echo  0 > /sys/class/net/eth0/rx_clock_rdy
    ifconfig eth0 down
    sleep 0.5
    vconfig add eth0 202
    sleep 0.1
    ifconfig eth0.202 192.168.202.1 netmask 255.255.255.0 broadcast 192.168.202.255 up
    sleep 0.1
    vconfig add eth0 205
    sleep 0.1
    ifconfig eth0.205 192.168.205.15 netmask 255.255.255.0 broadcast 192.168.205.255 up
    sleep 0.1
    ifconfig eth0:0 192.168.1.202 netmask 255.255.255.0 up
    sleep 0.1
    brctl delif bridge0 eth0
    # ip link add name bridge1 type bridge
    # sleep 0.1
    # ifconfig bridge1 192.168.1.202 netmask 255.255.255.255 broadcast 192.168.1.255 up
    # sleep 0.1
    # ip link add name bridge2 type bridge
    # sleep 0.1
    # ip link set eth0 master bridge2
    # sleep 0.1
    # ifconfig bridge2 192.168.1.86 netmask 255.255.255.0 broadcast 192.168.1.255 up
    # sleep 0.1
    echo  1 > /sys/class/net/eth0/rx_clock_rdy
    ifconfig eth0 up
    sleep 0.1
    /oemapp/scripts/monitor_dns_1.sh &
fi

allow_ip_init
sleep 0.1
allow_ip_check &

