cmake_minimum_required(VERSION 3.0)
project(carota_server)

# ==================== 参数配置 ====================
# parammanager 库路径（可以通过 -DPARAM_LIB_PATH=xxx 传入）
if(NOT DEFINED PARAM_LIB_PATH)
    # 默认使用本地 libs 目录
    set(PARAM_LIB_PATH ${PROJECT_SOURCE_DIR}/libs)
endif()

message(STATUS "Using parammanager library from: ${PARAM_LIB_PATH}")

# ==================== 头文件路径 ====================
include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/../cjson_com/inc
    ${PROJECT_SOURCE_DIR}/inc/mqtt
    ${PROJECT_SOURCE_DIR}/../external/civetweb/include
    ${PROJECT_SOURCE_DIR}/../../server/service/pm/inc
    ${PROJECT_SOURCE_DIR}/../mcuInterface/inc
    # 注意：parameter 的头文件已复制到 inc/ 目录下
)

# ==================== 查找 parammanager 库 ====================
find_library(PARAM_MANAGER_LIBRARY parammanager
    ${PARAM_LIB_PATH}
    ${CMAKE_INSTALL_PREFIX}/lib
    NO_DEFAULT_PATH
)

if(NOT PARAM_MANAGER_LIBRARY)
    message(FATAL_ERROR "parammanager library not found in ${PARAM_LIB_PATH} or ${CMAKE_INSTALL_PREFIX}/lib")
endif()

message(STATUS "PARAM_MANAGER_LIBRARY: ${PARAM_MANAGER_LIBRARY}")

# ==================== 可执行文件 ====================
add_executable(carota_server
    ${PROJECT_SOURCE_DIR}/../cjson_com/cJSON_Utils.c
    ${PROJECT_SOURCE_DIR}/../cjson_com/cJSON.c
    ${PROJECT_SOURCE_DIR}/src/ucu_status.c
    ${PROJECT_SOURCE_DIR}/src/carotsleephandle.c
    ${PROJECT_SOURCE_DIR}/src/ucu_upgrade.c
    ${PROJECT_SOURCE_DIR}/src/vehicle_status.c
    ${PROJECT_SOURCE_DIR}/src/carotaFilehandle.c
    ${PROJECT_SOURCE_DIR}/../mcuInterface/mcuinte.c
    ${PROJECT_SOURCE_DIR}/../mcuInterface/mcuinterbase.c
    ${PROJECT_SOURCE_DIR}/src/carotaHttpClient.c
    ${PROJECT_SOURCE_DIR}/src/carota_type.c
    ${PROJECT_SOURCE_DIR}/src/main.c
)

# ==================== 链接库 ====================
if (${MODEMTYPE} STREQUAL "AG591")
    target_link_libraries(carota_server
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
        pthread
        sqlite3
        sr-hal
        sr-ipc
        sr-sdk
        ${PROTOBUF_LIBRARY}
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
        ${ZLOG_LIBRARY}
        ${GLIB2_LIBRARY}
        ${GIO_LIBRARY}
        ${GOBJECT_LIBRARY}
        ${MQTT_LIBRARY}
        ${CIVETWEB}
        rt
        ssl
        dl
        glib-2.0
        crypto
        ssl
        curl
        ${PARAM_MANAGER_LIBRARY}
    )
endif()

if (${MODEMTYPE} STREQUAL "ag35cet")
    target_link_libraries(carota_server
        pthread
        rt
        sr-hal
        sr-ipc
        sr-sdk
        glib-2.0
        gio-2.0
        gobject-2.0
        ${ZLOG_LIBRARY}
        ${CRYPTO_LIBRARY}
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
    )
endif()

set_target_properties(carota_server PROPERTIES OUTPUT_NAME "carota_server")

# ==================== 安装目标 ====================
install(TARGETS carota_server DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)
