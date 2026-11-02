#!/bin/sh
echo "upgrade link ***********"


if [ ! -d  /mnt/carota/upgrade ];then
    mkdir -p  /mnt/carota/upgrade
fi

if [ ! -L /oemdata/upgrade ]; then
    ln -s /mnt/carota/upgrade /oemdata/upgrade
fi

if [ ! -d /oemdata/upgrade/download ];then
    mkdir -p /oemdata/upgrade/download
fi

if [ ! -d /oemdata/upgrade/ethernet ];then
    mkdir -p /oemdata/upgrade/ethernet
fi



echo "end !!!!!"