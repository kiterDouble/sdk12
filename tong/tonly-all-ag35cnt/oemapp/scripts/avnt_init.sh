#!/bin/sh
#
# 多拨号/VLAN桥接供网初始化脚本
#
# 功能概览：
#   1. ecm0 上划分 VLAN100/VLAN200 -> bridge100/bridge200
#   2. 给每个 bridge 配本地网关 IP (192.168.100.1 / 192.168.200.1)
#   3. ql_dnsmasq 给车机发 DHCP
#   4. mangle: 进/出方向打 fwmark(100/200)
#   5. policy routing:
#        mark=100  -> table 100 -> 走 ccinet1
#        mark=200  -> table 200 -> 走 ccinet3
#
# 依赖:
#   - vconfig / ifconfig / brctl / ip / iptables / ql_dnsmasq
#   - 已经存在拨号口: ccinet1 / ccinet2 / ccinet3
#   - 其他位置已经添加过 NAT:  ccinet1/2/3 MASQUERADE
#
# 注意:
#   - bridge100/bridge200 如果此时还没接上车机(usb0.100 / usb0.200)，
#     route 会显示 linkdown；这是正常的，不是本脚本的错误。
#

###############################################################################
# STEP 0: 基础准备
###############################################################################
mkdir -p /var/lib/misc
mkdir -p /var/run

# 开启内核 IPv4 转发（车机 LAN -> 蜂窝口 才会被转发）(父层脚本已执行此操作)
# echo 1 > /proc/sys/net/ipv4/ip_forward


###############################################################################
# STEP 1: VLAN & BRIDGE 基础网络结构
###############################################################################

# VLAN100 <-> bridge100
vconfig add ecm0 100           2>/dev/null
ifconfig ecm0.100 up

brctl addbr bridge100          2>/dev/null
brctl addif bridge100 ecm0.100 2>/dev/null
ifconfig bridge100 192.168.100.1 netmask 255.255.255.0 up

# VLAN200 <-> bridge200
vconfig add ecm0 200           2>/dev/null
ifconfig ecm0.200 up

brctl addbr bridge200          2>/dev/null
brctl addif bridge200 ecm0.200 2>/dev/null
ifconfig bridge200 192.168.200.1 netmask 255.255.255.0 up

# 注：
#  - 此时如果车机端(usb0.100 / usb0.200)还没起来，bridge100/200
#    在 ip route 里依然可能显示 linkdown，这是物理链路状态，不影响后续规则安装。
#  - 车机一旦枚举并拿到 DHCP，linkdown 会自动消失。

###############################################################################
# STEP 2: mangle 打标 (上行方向: 车机 -> 模块)
###############################################################################

# VLAN100 进来的流量 -> fwmark = 100
iptables -t mangle -C PREROUTING -i bridge100 -j MARK --set-mark 100 2>/dev/null || \
iptables -t mangle -A PREROUTING -i bridge100 -j MARK --set-mark 100

# VLAN200 进来的流量 -> fwmark = 200
iptables -t mangle -C PREROUTING -i bridge200 -j MARK --set-mark 200 2>/dev/null || \
iptables -t mangle -A PREROUTING -i bridge200 -j MARK --set-mark 200


###############################################################################
# STEP 3: Policy Routing (策略路由表)
###############################################################################
# 目标：
#   mark=100 的流量查 table 100：
#       default -> ccinet1
#       局域网 192.168.100.0/24 仍回 bridge100
#
#   mark=200 的流量查 table 200：
#       default -> ccinet2
#       局域网 192.168.200.0/24 仍回 bridge200
#
# 注意：
#   这里用 "default dev ccinetX" 而不是 "via GWx"，
#   因为蜂窝拨号接口(ccinetX)是点到点/直接路由，不一定有传统网关IP。

# 为 fwmark=100 指定 table 100
ip rule add fwmark 100 table 100 2>/dev/null

# table 100: 车机 VLAN100 -> ccinet1
# ip route add default dev ccinet1 table 100 2>/dev/null
ip route add 192.168.100.0/24 dev bridge100 table 100 2>/dev/null

# 为 fwmark=200 指定 table 200
ip rule add fwmark 200 table 200 2>/dev/null

# table 200: 车机 VLAN200 -> ccinet3
# ip route add default dev ccinet3 table 200 2>/dev/null
ip route add 192.168.200.0/24 dev bridge200 table 200 2>/dev/null

# 模块自身默认出口（本机进程用 ccinet3 走外部/私网）
# 如果系统里已经有默认路由，这里可以是 replace，防止多条 default 干扰
# ip route replace default dev ccinet3

###############################################################################
# STEP 4: 启动 DHCP 服务器和 DNS 代理服务器（dnsmasq）（ql_dnsmasq-> 移远）（注tbox已开启masq 0.0.0.0:53 dns解析代理）
###############################################################################

# 为 VLAN100 启动独立 dnsmasq 实例：
# - 绑定 bridge100 接口（不监听 lo）
# - 地址池：192.168.100.2 ~ 192.168.100.254，租期 43200 秒
# - DNS 下发 192.168.100.1（本机 bridge100 地址）
# - 使用 /tmp/resolv100.conf 作为上游 DNS 配置
# ql_dnsmasq --pid-file=/var/run/dnsmasq100.pid \
#   -i bridge100 -I lo -z \
#   --dhcp-range=bridge100,192.168.100.2,192.168.100.254,255.255.255.0,43200 \
#   --dhcp-option-force=6,192.168.100.1 \
#   --resolv-file=/tmp/resolv100.conf \
#   --user=root --group=root


# 为 VLAN200 启动独立 dnsmasq 实例：
# - 绑定 bridge200 接口（不监听 lo）
# - 地址池：192.168.200.2 ~ 192.168.200.254，租期 43200 秒
# - DNS 下发 192.168.200.1（本机 bridge200 地址）
# - 使用 /tmp/resolv200.conf 作为上游 DNS 配置
# ql_dnsmasq --pid-file=/var/run/dnsmasq200.pid \
#   -i bridge200 -I lo -z \
#   --dhcp-range=bridge200,192.168.200.2,192.168.200.254,255.255.255.0,43200 \
#   --dhcp-option-force=6,192.168.200.1 \
#   --resolv-file=/tmp/resolv200.conf \
#   --user=root --group=root

###############################################################################
# STEP 5: 说明 / 调试建议
###############################################################################
# 1. 现在不要担心 "ip route show" 里 bridge100/200 仍显示 linkdown。
#    车机(usb0.100 / usb0.200)一旦真的连上并拿到 DHCP，link 会自动转为 UP。
#
# 2. 车机连上 VLAN100 后：
#       - 它拿到 192.168.100.x
#       - 它发流量 -> 进 bridge100 -> 被打 mark=100
#       - rule fwmark 100 -> table 100
#       - table100 default dev ccinet1
#       - 出去时 NAT 已在别处MASQUERADE到 ccinet1 (你系统已有)
#
#    用 tcpdump 验证：
#       tcpdump -i ccinet1 icmp
#
# 3. VLAN200 同理走 ccinet3：
#       tcpdump -i ccinet3 icmp
#
# 4. 模块自己上的业务（不是车机过来的）默认走 ccinet3，因为我们用
#       ip route replace default dev ccinet3
#
# 5. IPv4 转发我们已经 echo 1 > /proc/sys/net/ipv4/ip_forward
#    如果系统重启后需要常驻，记得固化到 sysctl.conf 或开机脚本。

# echo 1 > /proc/sys/net/ipv4/ip_forward
# iptables -t nat -A POSTROUTING -o ccinet1 -j MASQUERADE
# iptables -t nat -A POSTROUTING -o ccinet2 -j MASQUERADE
# iptables -t nat -A POSTROUTING -o ccinet3 -j MASQUERADE