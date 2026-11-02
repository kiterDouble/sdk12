cmake_minimum_required(VERSION 3.0)
project(tspApp)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++11 -D_GNU_SOURCE -Wno-deprecated -Wno-psabi")

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/
    ${PROJECT_SOURCE_DIR}/../../common_inc/
)

add_executable(tspApp
        "${PROJECT_SOURCE_DIR}/json/src/json_reader.cpp"
        "${PROJECT_SOURCE_DIR}/json/src/json_value.cpp"
        "${PROJECT_SOURCE_DIR}/json/src/json_writer.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_crc32.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_connect.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_common.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_oder_charging.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_main.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_log_report.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_gettoken.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_dispatch.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_cache_queue.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_monitor.cpp"
#        "${PROJECT_SOURCE_DIR}/src/tbox_uds_tp.cpp"
#        "${PROJECT_SOURCE_DIR}/src/tbox_uds_service.cpp"
        "${PROJECT_SOURCE_DIR}/src/task_exc_handle.cpp"
        "${PROJECT_SOURCE_DIR}/src/dns.cpp"
        "${PROJECT_SOURCE_DIR}/src/tsp_parser_diagnosis.cpp"
)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce")
target_link_libraries(tspApp
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
target_link_libraries(tspApp
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    pthread
    sqlite3
    sr-hal
    sr-ipc
    sr-sdk
    sr-comfunc
    ${GNHTTP2_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    ${CURL_LIBRARY}
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${MQTT_LIBRARY}
    rt
)

get_target_property(linked_libraries tspApp INTERFACE_LINK_LIBRARIES)
message("Linked libraries: ${linked_libraries}")

endif()

if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(tspApp
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    pthread
    sqlite3
    sr-hal
    sr-ipc
    sr-sdk
    sr-comfunc
    ${GNHTTP2_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    ${CURL_LIBRARY}
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${MQTT_LIBRARY}
    rt
)

get_target_property(linked_libraries tspApp INTERFACE_LINK_LIBRARIES)
message("Linked libraries: ${linked_libraries}")

endif()

# 设置可执行文件名称
set_target_properties(tspApp PROPERTIES OUTPUT_NAME "tspApp")

# 安装目标
install(TARGETS tspApp DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

