#!/bin/sh

brctl delif bridge0 eth0
brctl addbr br-lan0
brctl addif br-lan0 eth0

ifconfig br-lan0 hw ether 02:47:57:4D:00:97 up
ifconfig eth0 up

sleep 0.5

ip6tables -A INPUT -p ipv6-icmp -j DROP
ip6tables -A OUTPUT -p ipv6-icmp -j DROP
ip6tables -A FORWARD -p ipv6-icmp -j DROP


# ================= VLAN 4 : 172.16.64.45 =================
vconfig add br-lan0 4
ifconfig br-lan0.4 172.16.64.45 netmask 255.255.240.0

ip neigh add 172.16.64.43  lladdr 02:47:00:00:00:2B dev br-lan0.4 nud permanent
ip neigh add 172.16.64.48  lladdr 02:47:00:00:00:30 dev br-lan0.4 nud permanent
ip neigh add 172.16.64.79  lladdr 02:47:00:00:00:4F dev br-lan0.4 nud permanent
ip neigh add 172.16.64.64  lladdr 02:47:00:00:00:40 dev br-lan0.4 nud permanent
ip neigh add 172.16.64.159 lladdr 02:47:00:00:00:9F dev br-lan0.4 nud permanent

# ifconfig br-lan0.4 -arp


# ================= VLAN 6 : 172.16.96.45 =================
vconfig add br-lan0 6
ifconfig br-lan0.6 172.16.96.45 netmask 255.255.240.0

ip neigh add 172.16.96.43 lladdr 02:47:00:00:00:2B dev br-lan0.6 nud permanent
ip neigh add 172.16.66.79 lladdr 02:47:00:00:02:4F dev br-lan0.6 nud permanent
ip neigh add 172.16.70.14 lladdr 02:47:00:00:06:0E dev br-lan0.6 nud permanent

# ifconfig br-lan0.6 -arp


# ================= VLAN 7 : 172.16.112.45 =================
vconfig add br-lan0 7
ifconfig br-lan0.7 172.16.112.45 netmask 255.255.240.0

ip neigh add 172.16.112.79 lladdr 02:47:00:00:00:4F dev br-lan0.7 nud permanent

# ifconfig br-lan0.7 -arp


# ================= VLAN 8 : 172.16.128.45 =================
vconfig add br-lan0 8
ifconfig br-lan0.8 172.16.128.45 netmask 255.255.240.0

ip neigh add 172.16.128.14 lladdr 02:47:00:00:00:0E dev br-lan0.8 nud permanent
ip neigh add 172.16.128.43 lladdr 02:47:00:00:00:2B dev br-lan0.8 nud permanent
ip neigh add 172.16.128.64 lladdr 02:47:00:00:00:40 dev br-lan0.8 nud permanent
ip neigh add 172.16.128.57 lladdr 02:47:00:00:00:39 dev br-lan0.8 nud permanent

# ifconfig br-lan0.8 -arp


# ================= VLAN 9 : 172.16.144.45 =================
vconfig add br-lan0 9
ifconfig br-lan0.9 172.16.144.45 netmask 255.255.240.0

ip neigh add 172.16.144.14 lladdr 02:47:00:00:00:0E dev br-lan0.9 nud permanent
ip neigh add 172.16.144.43 lladdr 02:47:00:00:00:2B dev br-lan0.9 nud permanent
ip neigh add 172.16.144.57 lladdr 02:47:00:00:00:39 dev br-lan0.9 nud permanent

# ifconfig br-lan0.9 -arp


# ================= VLAN 19 : 172.17.48.45 =================
vconfig add br-lan0 19
ifconfig br-lan0.19 172.17.48.45 netmask 255.255.240.0

ip neigh add 172.17.48.43 lladdr 02:47:00:00:00:2B dev br-lan0.19 nud permanent
ip neigh add 172.17.54.14 lladdr 02:47:00:00:06:0E dev br-lan0.19 nud permanent

# ifconfig br-lan0.19 -arp


# ================= TTL control =================
iptables -t mangle -A POSTROUTING -o br-lan0.4  -j TTL --ttl-set 128
iptables -t mangle -A POSTROUTING -o br-lan0.6  -j TTL --ttl-set 128
iptables -t mangle -A POSTROUTING -o br-lan0.7  -j TTL --ttl-set 128
iptables -t mangle -A POSTROUTING -o br-lan0.8  -j TTL --ttl-set 128
iptables -t mangle -A POSTROUTING -o br-lan0.9  -j TTL --ttl-set 128
iptables -t mangle -A POSTROUTING -o br-lan0.19 -j TTL --ttl-set 128


# ================= Bring up =================
ifconfig br-lan0.4 up
ifconfig br-lan0.6 up
ifconfig br-lan0.7 up
ifconfig br-lan0.8 up
ifconfig br-lan0.9 up
ifconfig br-lan0.19 up


sysctl -w net.ipv4.icmp_echo_ignore_broadcasts=0
iptables -I OUTPUT -p icmp --icmp-type echo-reply -j ACCEPT

iptables -A OUTPUT -p igmp -j DROP
iptables -A INPUT  -p igmp -j DROP
iptables -A FORWARD -p igmp -j DROP
