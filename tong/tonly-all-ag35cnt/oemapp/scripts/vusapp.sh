#!/bin/sh

VUSAPP_DIR=/oemapp/gac/bin/abup/vus/bin
VUSLIB_DIR=/oemapp/gac/bin/abup/vus/lib/
APPOUT_DIR=/mnt/sdcard/tmplog/ota
TBOXLIB_DIR=/oemapp/gac/lib/
OTAM_DAATA_DIR=/mnt/sdcard/data/ota
LOGFILE="/oemdata/log/sr.log"
APP="${VUSAPP_DIR}/admfotaapp"
CONF="${VUSAPP_DIR}/ucmconfig.ini"

if [ ! -d $APPOUT_DIR ];then
    mkdir -p $APPOUT_DIR
fi

if [ ! -d $OTAM_DAATA_DIR ];then
    mkdir -p $OTAM_DAATA_DIR
fi

export LD_LIBRARY_PATH=${TBOXLIB_DIR}:${VUSLIB_DIR}

if [ ! -d "$APPOUT_DIR" ]; then
    mkdir -p $APPOUT_DIR
fi

RunOtaMasterApp()
{
    while true
    do
        find $APPOUT_DIR -name sh_start_rec.log -size +10k | xargs rm -f
        echo "[`cat /proc/uptime | cut -d" " -f1`] system start time">> ${APPOUT_DIR}/sh_start_rec.log
        time=$(date "+%Y-%m-%d %H:%M:%S")
        echo "[$time] ota start time">> ${APPOUT_DIR}/sh_start_rec.log

        "$APP" -c "$CONF" > /dev/null 2>&1
        date >> "$LOGFILE"
        echo "\"$APP -c $CONF\" exit." >> "$LOGFILE"
        sleep 1
    done
}

RunOtaMasterApp &
