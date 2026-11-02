#!/bin/sh

LOGFILE="/oemdata/log/sr.log"

SEVERLIST="/oemapp/servers/mcuServer 
           /oemapp/servers/modemServer
           /oemapp/servers/gnssServer
           /oemapp/servers/eraServer
           /oemapp/servers/netServer
           /oemapp/servers/pmServer
           /oemapp/servers/ftyServer
           /oemapp/servers/paramServer
           /oemapp/servers/timeServer
           /oemapp/servers/signalServer"

RunServer() {
    while true
    do
        $@ > /dev/null
        date >> ${LOGFILE}
        echo "\"$@\" exit." >> ${LOGFILE}
        sleep 1
    done
}

for line in $SEVERLIST; do
    RunServer ${line} &
done