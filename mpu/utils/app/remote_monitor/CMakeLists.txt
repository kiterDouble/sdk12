cmake_minimum_required(VERSION 3.0)
project(remote_monitor)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/cjson
    #${PROJECT_SOURCE_DIR}/inc/json
    ${PROJECT_SOURCE_DIR}/inc/http
    ${PROJECT_SOURCE_DIR}/inc/misc
    ${PROJECT_SOURCE_DIR}/inc/mqtt
    ${PROJECT_SOURCE_DIR}/inc/can
    ${PROJECT_SOURCE_DIR}/inc/udp
    ${PROJECT_SOURCE_DIR}/inc/misc/cycle/
)

add_executable(remote_monitor

        #"${PROJECT_SOURCE_DIR}/src/misc/cycle/mqtt_proto_package.cpp"
        "${PROJECT_SOURCE_DIR}/src/misc/cycle/cycle_get_mcu_version.cpp"
        "${PROJECT_SOURCE_DIR}/src/misc/cycle/cycle_get_modem_info.cpp"
        "${PROJECT_SOURCE_DIR}/src/misc/cycle/cycle_evet_handle.cpp"
        "${PROJECT_SOURCE_DIR}/src/misc/cycle/cycle_misc_json_interface.cpp"
        "${PROJECT_SOURCE_DIR}/src/misc/cycle/cycle_common.cpp"
        

        "${PROJECT_SOURCE_DIR}/src/json/json_reader.cpp"
        "${PROJECT_SOURCE_DIR}/src/json/json_value.cpp"
        "${PROJECT_SOURCE_DIR}/src/json/json_writer.cpp"

        "${PROJECT_SOURCE_DIR}/src/cjson/cJSON.c"
        "${PROJECT_SOURCE_DIR}/src/cjson/cjson_interface.cpp"
        "${PROJECT_SOURCE_DIR}/src/http/http.cpp" 
        "${PROJECT_SOURCE_DIR}/src/mqtt/mqtt.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt/mqtt_data_handler.cpp"
        "${PROJECT_SOURCE_DIR}/src/misc/queue.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt/mqtt_thread.cpp"
        "${PROJECT_SOURCE_DIR}/src/misc/modem_function.cpp"
        "${PROJECT_SOURCE_DIR}/src/misc/system_interface.cpp" 
        "${PROJECT_SOURCE_DIR}/src/misc/thread_interface.cpp" 
        "${PROJECT_SOURCE_DIR}/src/misc/mcu_interface.cpp" 
        "${PROJECT_SOURCE_DIR}/src/misc/completion_timeout.cpp" 
        "${PROJECT_SOURCE_DIR}/src/misc/string_hex_interface.cpp" 
        "${PROJECT_SOURCE_DIR}/src/misc/timer_interface.cpp" 
        "${PROJECT_SOURCE_DIR}/src/can/mqtt_can_upload.cpp" 
        "${PROJECT_SOURCE_DIR}/src/udp/udp_client.cpp" 
        #"${PROJECT_SOURCE_DIR}/src/udp/udp_server.cpp" 
        
        "${PROJECT_SOURCE_DIR}/src/main.cpp"
)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce")
target_link_libraries(mqtt
    pthread
    sqlite3
    sr-hal
    sr-ipc
    sr-sdk
    rt
    glib-2.0
    gio-2.0
    gobject-2.0
    protobuf
    curl
    paho-mqtt3cs
    paho-mqtt3as
    pki_sdk
    ${ZLOG_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
)
endif()


if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(remote_monitor
    pthread
    sqlite3
    rt
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    pki_sdk
    sr-comfunc
    curl
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    ${MQTT_ASYNC_LIBRARY}
    rt

)
endif()
# 设置可执行文件名称
set_target_properties(remote_monitor PROPERTIES OUTPUT_NAME "remote_monitor")

# 安装目标
install(TARGETS remote_monitor  DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

