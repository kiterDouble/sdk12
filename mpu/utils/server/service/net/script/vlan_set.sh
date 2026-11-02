#!/bin/sh

# 检查两个 IP 是否在同一个子网内的函数
# 参数: <ip1> <ip2>
# 返回值: 如果 IP 在同一个子网内返回 0，否则返回 1
function check_same_subnet() {
    local ip1=$1
    local ip2=$2

    # 从 IP 中提取网络部分
    net1=$(echo "$ip1" | cut -d'.' -f1-3)
    net2=$(echo "$ip2" | cut -d'.' -f1-3)

    # 比较网络部分
    if [ "$net1" == "$net2" ]; then
        return 0
    else
        return 1
    fi
}

# 检查脚本是否以 root 身份运行
if [ "$(id -u)" -ne "0" ]; then
    echo "此脚本必须以 root 身份运行" 1>&2
    exit 1
fi

# 检查是否提供了正确数量的参数
if [ "$#" -ne 3 ]; then
    echo "用法: $0 <vlan_count> <local_ip> <remote_ip>" 1>&2
    exit 1
fi

vlan_count=$1
local_ip=$2
remote_ip=$3

# 检查本地和远程 IP 是否在同一个子网内
if ! check_same_subnet "$local_ip" "$remote_ip"; then
    echo "错误: 本地和远程 IP 不在同一个子网内" 1>&2
    exit 1
fi

# 从本地 IP 提取前两个八位字节
local_first_two_octets=$(echo $local_ip | cut -d'.' -f1-2)

# 从本地 IP 提取第三个八位字节
local_third_octet=$(echo $local_ip | cut -d'.' -f3)

local_last_octet=$(echo $local_ip | cut -d'.' -f4)

# 从远程 IP 提取前两个八位字节
remote_first_two_octets=$(echo $remote_ip | cut -d'.' -f1-2)

# 从远程 IP 提取第三个八位字节
remote_third_octet=$(echo $remote_ip | cut -d'.' -f3)

remote_last_octet=$(echo $remote_ip | cut -d'.' -f4)

# 添加 VLAN 接口
# 添加 VLAN 接口
i=0
while [ $i -lt $vlan_count ]; do
    local_bridge_ip="$local_first_two_octets.$((local_third_octet + i)).$local_last_octet"
    remote_bridge_ip="$remote_first_two_octets.$((remote_third_octet + i)).$remote_last_octet"

    vconfig add ecm0 $((local_third_octet + i))
    ifconfig ecm0.$((local_third_octet + i)) up

    bridge_name="bridge$((local_third_octet + i))"
    brctl addbr $bridge_name
    ifconfig $bridge_name $local_bridge_ip netmask 255.255.255.0 up
    brctl addif $bridge_name ecm0.$((local_third_octet + i))

    # 为每个网桥启动 DHCP 服务
    dnsmasq --pid-file=/var/run/dnsmasq$((local_third_octet + i)).pid -i $bridge_name -I lo -z --dhcp-range=$bridge_name,$remote_bridge_ip,$local_first_two_octets.$((local_third_octet + i)).254,255.255.255.0,43200 --dhcp-option-force=6,$local_bridge_ip --resolv-file=/etc/resolve.conf

    i=$((i + 1))
done

echo "VLAN 配置成功完成。"
