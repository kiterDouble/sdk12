#!/bin/sh

LOGFILE="/oemdata/log/sr.log"

SEVERLIST="/oemapp/servers/propertyServer
           /oemapp/servers/cpuMonitorServer
           /oemapp/servers/mcuServer
           /oemapp/servers/modemServer
           /oemapp/servers/netServer
           /oemapp/servers/gnssServer
           /oemapp/servers/pmServer
           /oemapp/servers/ftyServer
           /oemapp/servers/paramServer
           /oemapp/servers/timeServer
           /oemapp/servers/signalServer
           /oemapp/servers/hsmServer
           /oemapp/servers/logPackServer
           /oemapp/servers/canLoggerServer"

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
