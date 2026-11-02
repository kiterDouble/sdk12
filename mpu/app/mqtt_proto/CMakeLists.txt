cmake_minimum_required(VERSION 3.0)
project(mqtt_proto)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++11 -D_GNU_SOURCE -Wno-deprecated -Wno-psabi")

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/inc/mqtt/
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/../libexternal/
    ${PROJECT_SOURCE_DIR}/../libexternal/proto_image/
    ${PROJECT_SOURCE_DIR}/../audio/include/
)

add_executable(mqtt_proto
        "${PROJECT_SOURCE_DIR}/src/mqtt_common.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_connect.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_dispatch.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_evet_handle.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_gettoken.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_main.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_proto_package.cpp"
        "${PROJECT_SOURCE_DIR}/src/task_exc_handle.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_get_modem_info.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_gateway.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_srlog_upload.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_get_mcu_version.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_qos.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_emmc_ro.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_log_coord.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_log_sync.cpp"
        "${PROJECT_SOURCE_DIR}/src/mqtt_queue.c"
        "${PROJECT_SOURCE_DIR}/json/src/json_reader.cpp"
        "${PROJECT_SOURCE_DIR}/json/src/json_value.cpp"
        "${PROJECT_SOURCE_DIR}/json/src/json_writer.cpp"
)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce")
target_link_libraries(mqtt_proto
    pthread
    sqlite3
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    ${ZLOG_LIBRARY}
)
endif()
if (${MODEMTYPE} STREQUAL "ag35cet")
target_link_libraries(mqtt_proto
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    pthread
    sqlite3
    sr-hal
    sr-ipc
    sr-sdk
    sr-comfunc
    ${PROTOBUF_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    curl
    ${CIVETWEB}
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${MQTT_LIBRARY}
    rt
)

get_target_property(linked_libraries mqtt_proto INTERFACE_LINK_LIBRARIES)
message("Linked libraries: ${linked_libraries}")

endif()


if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(mqtt_proto
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    pthread
    sqlite3
    sr-hal
    sr-ipc
    sr-sdk
    sr-comfunc
    pki_sdk
    ql_sdk
    ${PROTOBUF_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    curl
    ${CIVETWEB}
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${MQTT_LIBRARY}
    rt
)

get_target_property(linked_libraries mqtt_proto INTERFACE_LINK_LIBRARIES)
message("Linked libraries: ${linked_libraries}")

endif()


# 设置可执行文件名称
set_target_properties(mqtt_proto PROPERTIES OUTPUT_NAME "mqtt_proto")

# 安装目标
install(TARGETS mqtt_proto DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

