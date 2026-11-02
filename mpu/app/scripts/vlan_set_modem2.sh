#!/bin/sh

# 检查两个 IP 是否在同一个子网内的函数
function check_same_subnet() {
    local ip1=$1
    local ip2=$2
    net1=$(echo "$ip1" | cut -d'.' -f1-3)
    net2=$(echo "$ip2" | cut -d'.' -f1-3)
    if [ "$net1" == "$net2" ]; then
        return 0
    else
        return 1
    fi
}

# 检查 root 权限
if [ "$(id -u)" -ne "0" ]; then
    echo "此脚本必须以 root 身份运行" 1>&2
    exit 1
fi

# 检查参数数量
if [ "$#" -ne 3 ]; then
    echo "用法: $0 <vlan_count> <local_ip> <remote_ip>" 1>&2
    exit 1
fi

# 检查 /oemdata 目录
if [ ! -d "/oemdata" ] || [ ! -w "/oemdata" ]; then
    echo "错误: /oemdata 目录不存在或不可写" 1>&2
    exit 1
fi

# 检查 /etc/resolv.conf
if [ ! -f "/etc/resolv.conf" ]; then
    echo "错误: /etc/resolv.conf 不存在，请配置 DNS" 1>&2
    exit 1
fi

vlan_count=$1
local_ip=$2
remote_ip=$3

# 检查子网
if ! check_same_subnet "$local_ip" "$remote_ip"; then
    echo "错误: 本地和远程 IP 不在同一个子网内" 1>&2
    exit 1
fi

# 提取 IP 部分
local_first_two_octets=$(echo $local_ip | cut -d'.' -f1-2)
local_third_octet=$(echo $local_ip | cut -d'.' -f3)
local_last_octet=$(echo $local_ip | cut -d'.' -f4)
remote_first_two_octets=$(echo $remote_ip | cut -d'.' -f1-2)
remote_third_octet=$(echo $remote_ip | cut -d'.' -f3)
remote_last_octet=$(echo $remote_ip | cut -d'.' -f4)

# 检查拨号网卡状态
dialup_interface="rmnet_data0"
if ip link show rmnet_data0 > /dev/null 2>&1 && ip addr show rmnet_data0 | grep -q "inet "; then
    echo "使用拨号网卡: $dialup_interface"
else
    echo "警告: 拨号网卡 rmnet_data0 未正确配置或未连接，将继续配置 VLAN，但 NAT 规则可能无效"
fi

# 启用 IP 转发
echo 1 > /proc/sys/net/ipv4/ip_forward
if [ "$(cat /proc/sys/net/ipv4/ip_forward)" -ne 1 ]; then
    echo "错误: 无法启用 IP 转发" 1>&2
    exit 1
fi

# 清理旧的 NAT 规则
iptables -t nat -F POSTROUTING
iptables -t nat -X

# 检查并清理占用 local_ip 的进程
for port in 53 67 68; do
    pid=$(netstat -tulnp 2>/dev/null | grep "$local_ip:$port" | awk '{print $7}' | cut -d'/' -f1)
    if [ -n "$pid" ]; then
        echo "发现占用 $local_ip:$port 的进程 (PID: $pid)，正在终止..."
        kill -9 $pid
    fi
done

# 清理旧的 VLAN 和网桥
i=0
while [ $i -lt $vlan_count ]; do
    bridge_name="bridge$((local_third_octet + i))"
    ip link delete $bridge_name 2>/dev/null
    ip link delete eth0.$((local_third_octet + i)) 2>/dev/null
    if [ -f "/oemdata/dnsmasq$((local_third_octet + i)).pid" ]; then
        kill $(cat /oemdata/dnsmasq$((local_third_octet + i)).pid 2>/dev/null) 2>/dev/null
        rm -f /oemdata/dnsmasq$((local_third_octet + i)).pid
    fi
    i=$((i + 1))
done

# 添加 VLAN 接口
i=0
while [ $i -lt $vlan_count ]; do
    local_bridge_ip="$local_first_two_octets.$((local_third_octet + i)).$local_last_octet"
    remote_bridge_ip="$remote_first_two_octets.$((remote_third_octet + i)).$remote_last_octet"

    # 检查 VLAN 接口
    if ip link show eth0.$((local_third_octet + i)) > /dev/null 2>&1; then
        echo "警告: VLAN 接口 eth0.$((local_third_octet + i)) 已存在，跳过创建"
    else
        if ! vconfig add eth0 $((local_third_octet + i)); then
            echo "错误: 创建 VLAN 接口 eth0.$((local_third_octet + i)) 失败" 1>&2
            exit 1
        fi
        if ! ifconfig eth0.$((local_third_octet + i)) up; then
            echo "错误: 启用 VLAN 接口 eth0.$((local_third_octet + i)) 失败" 1>&2
            exit 1
        fi
    fi

    bridge_name="bridge$((local_third_octet + i))"

    # 检查网桥
    if ip link show $bridge_name > /dev/null 2>&1; then
        echo "警告: 网桥 $bridge_name 已存在，跳过创建"
    else
        if ! brctl addbr $bridge_name; then
            echo "错误: 创建网桥 $bridge_name 失败" 1>&2
            exit 1
        fi
        if ! ifconfig $bridge_name $local_bridge_ip netmask 255.255.255.0 up; then
            echo "错误: 配置网桥 $bridge_name IP 失败" 1>&2
            exit 1
        fi
        if ! brctl addif $bridge_name eth0.$((local_third_octet + i)); then
            echo "错误: 将 eth0.$((local_third_octet + i)) 添加到网桥 $bridge_name 失败" 1>&2
            exit 1
        fi
    fi

    # 添加 NAT 规则
    if ! iptables -t nat -A POSTROUTING -s $local_first_two_octets.$((local_third_octet + i)).0/24 -o $dialup_interface -j MASQUERADE; then
        echo "错误: 添加 NAT 规则失败" 1>&2
    fi
    
    #清理移远相关占用dnsmasq进程
    kill -9 $(ps | grep ql_dnsmasq | grep -v grep | awk '{print $1}')

    # 启动 DHCP 服务
    if ! dnsmasq --pid-file=/oemdata/dnsmasq$((local_third_octet + i)).pid \
            -i $bridge_name -I lo -z \
            --dhcp-range=$bridge_name,$remote_bridge_ip,$local_first_two_octets.$((local_third_octet + i)).254,255.255.255.0,43200 \
            --dhcp-option-force=6,$local_bridge_ip \
            --resolv-file=/etc/resolv.conf \
            --dhcp-leasefile=/oemdata/dnsmasq.leases.$((local_third_octet + i)); then
        echo "错误: 启动 dnsmasq 服务失败" 1>&2
        exit 1
    fi

    i=$((i + 1))
done

# 如果 local_ip 是 192.168.1.1，添加过滤规则并执行 VLAN down/up
if [ "$local_first_two_octets.$local_third_octet.$local_last_octet" = "192.168.1.1" ]; then

    # 找到与 192.168.1.1 关联的 VLAN 接口并执行 down 操作
    target_vlan_id=$local_third_octet
    target_bridge="bridge$local_third_octet"
    echo "对 VLAN 接口 eth0.$target_vlan_id 和网桥 $target_bridge 执行 down/up 操作..."
    if ! ip link set eth0.$target_vlan_id down; then
        echo "错误: 禁用 VLAN 接口 eth0.$target_vlan_id 失败" 1>&2
        exit 1
    fi
    if ! ip link set $target_bridge down; then
        echo "错误: 禁用网桥 $target_bridge 失败" 1>&2
        exit 1
    fi
    echo "VLAN 接口 eth0.$target_vlan_id 和网桥 $target_bridge 已关闭"
fi

echo "VLAN 配置成功完成。"