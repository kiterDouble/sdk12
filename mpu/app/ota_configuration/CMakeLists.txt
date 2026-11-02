cmake_minimum_required(VERSION 3.0)
project(ota_configuration)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/cjson
    ${PROJECT_SOURCE_DIR}/inc/http
    ${PROJECT_SOURCE_DIR}/inc/misc
    ${PROJECT_SOURCE_DIR}/inc/mqtt
    ${PROJECT_SOURCE_DIR}/inc/udp
)

add_executable(ota_configuration
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
        "${PROJECT_SOURCE_DIR}/src/misc/completion_timeout.cpp" 
        "${PROJECT_SOURCE_DIR}/src/misc/mcu_interface.cpp" 
        "${PROJECT_SOURCE_DIR}/src/misc/string_hex_interface.cpp" 
        "${PROJECT_SOURCE_DIR}/src/misc/timer_interface.cpp" 
        "${PROJECT_SOURCE_DIR}/src/udp/udp_server.cpp" 
        
        "${PROJECT_SOURCE_DIR}/src/main.cpp"
)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce")
target_link_libraries(ota_configuration
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
    
    ${ZLOG_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
)
endif()
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(ota_configuration
    pthread
    sqlite3
    rt
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    pki_sdk
    curl
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    ${MQTT_ASYNC_LIBRARY}
    

)
endif()
# 设置可执行文件名称
set_target_properties(ota_configuration PROPERTIES OUTPUT_NAME "ota_configuration")

# 安装目标
install(TARGETS ota_configuration  DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

