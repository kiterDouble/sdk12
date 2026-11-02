#!/bin/sh
LOGFILE="/oemdata/log/sr.log"

APPLIST="/oemapp/apps/tspApp
        /oemapp/apps/dtcApp
        /oemapp/apps/terminal_manager"

RunApp()
{
    while true
    do
    $@ > /dev/null
    date >> ${LOGFILE}
    echo "\"$@\" exit." >> ${LOGFILE}
    sleep 1
    done
}

for line in $APPLIST; do
    RunApp ${line} &
done
