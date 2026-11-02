#!/bin/bash
LOGFILE="/oemdata/log/sr.log"

APPLIST=("/oemapp/apps/mqtt_proto"
        "/oemapp/apps/pki_cert"
        "/oemapp/apps/can_file_save"
        "/oemapp/apps/remote_monitor"

            )

if [ -e "/dev/mmcblk0" ]; then
    #APPLIST+=("/oemapp/apps/can_file_save")
    APPLIST+=("/oemapp/apps/audio_app")
    APPLIST+=("/oemapp/apps/carota_server")
    APPLIST+=("/oemapp/apps/ota_configuration")
	# APPLIST+=("/oemapp/apps/modem_a")    
else
	APPLIST+=("/oemapp/apps/modem_b")
    APPLIST+=("/oemapp/apps/carota_server")
    APPLIST+=("/oemapp/apps/ota_configuration")
fi

RunApp()
{
    while true
    do
    value=`echo $@`
    #if [[ "$value" == "/oemapp/apps/someipClient" ]];then
    #    VSOMEIP_CONFIGURATION=/oemdata/config/someip/client_vsomeip.json /oemapp/apps/someipClient > /dev/null
    #elif [[ "$value" == "/oemapp/apps/someipServer" ]];then
    #    VSOMEIP_CONFIGURATION=/oemdata/config/someip/server_vsomeip.json /oemapp/apps/someipServer > /dev/null
    #else
        $@ > /dev/null
    #fi
    date >> ${LOGFILE}
    echo "\"$@\" exit." >> ${LOGFILE}
#    sleep 1
    done
}

for line in "${APPLIST[@]}"; do
    RunApp ${line} &
    sleep 2
done

