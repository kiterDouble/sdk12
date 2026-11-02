#/usr/bin/bash

compile_mqtt() {
    rm -rf build
    mkdir build
    if [ "${HOST}" = "arm-openwrt-linux-muslgnueabi" ]; then
        sed -i.bak 's/ -lpthread//g' Makefile
        make CFLAGS="-I/opt/A7805/A7805lib/include/" LDFLAGS="-L/opt/A7805/A7805lib/lib/ -lssl -lcrypto"
        mv Makefile.bak Makefile
    else
        make
    fi
    mkdir -p ../thrid-lib-out/include/paho.mqtt.c/
    cp  src/MQTTAsync.h 			../thrid-lib-out/include/paho.mqtt.c/
    cp  src/MQTTClient.h 			../thrid-lib-out/include/paho.mqtt.c/
    cp  src/MQTTClientPersistence.h 	../thrid-lib-out/include/paho.mqtt.c/
    cp  src/MQTTProperties.h 	../thrid-lib-out/include/paho.mqtt.c/
    cp  src/MQTTExportDeclarations.h 	../thrid-lib-out/include/paho.mqtt.c/
    cp  src/MQTTPersistence.h  		../thrid-lib-out/include/paho.mqtt.c/
    cp  src/MQTTReasonCodes.h 		../thrid-lib-out/include/paho.mqtt.c/
    cp  src/MQTTSubscribeOpts.h 		../thrid-lib-out/include/paho.mqtt.c/
    cp -a build/output/libpaho-mqtt3* 	../thrid-lib-out/lib/
}

compile_mqtt
