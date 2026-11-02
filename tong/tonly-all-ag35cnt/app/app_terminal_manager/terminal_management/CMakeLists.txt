cmake_minimum_required(VERSION 3.0)
project(terminal_management)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/private
    ${PROJECT_SOURCE_DIR}/inc/public
    ${PROJECT_SOURCE_DIR}/tspFW/inc
    ${PROJECT_SOURCE_DIR}/tspProto
    ${PROJECT_SOURCE_DIR}/sm/inc
    ${SDKTARGETSYSROOT}/usr/include/glib-2.0
    ${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include
    ${PROJECT_SOURCE_DIR}/../../lib/include
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/C3BaseLineOut/thrid-lib-out/include/flatcc/portable
    # ${PROJECT_SOURCE_DIR}/../../../app/hsm_rpc_server/src/HedSDK/extern
)

file(GLOB TSP_FW_SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tspFW/src/*.c
)
file(GLOB S3PROTO_SRC_FILES
    ${PROJECT_SOURCE_DIR}/tspProto/net/sirun/tboxs3/connect/*.pb.cc
)
list(REMOVE_ITEM S3PROTO_SRC_FILES
    ${PROJECT_SOURCE_DIR}/tspProto/net/sirun/tboxs3/connect/upgrade.pb.cc # 暂不使用该文件
)
file(GLOB SM_SRC_FILES
    ${PROJECT_SOURCE_DIR}/sm/src/*.c
)

set(SRC_LIST
    src/terminal_management_api.cpp
    src/srTspEncryptAndDecrypt.cpp
    src/terminal_management_data_packag.cpp
    src/terminal_management_data_monitor.cpp
    src/terminal_management_data_report.cpp
    src/terminal_management_data_response.cpp
    src/terminal_management_data_encode.cpp
    src/srTspRemoteProcess.cpp
    src/srTspStateMachine.c
    src/srTspCallbackRegister.c
    src/srTspLogin.cpp
    # ${PROJECT_SOURCE_DIR}/../../../app/hsm_rpc_server/src/HedSDK/extern/sm4.c
    # ${PROJECT_SOURCE_DIR}/../../../app/hsm_rpc_server/src/HedSDK/extern/HED_SM4_Soft.c
    # ${PROJECT_SOURCE_DIR}/../../../app/hsm_rpc_server/src/HedSDK/extern/HED_SymmetricAlgModeProcess.c
    ${SM_SRC_FILES}
    ${TSP_FW_SRC_FILES}
    ${S3PROTO_SRC_FILES}
)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})

target_link_directories(${PROJECT_NAME} PRIVATE 
    /oemapp/lib
)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}/inc/public/
    ${PROJECT_SOURCE_DIR}/tspFW/inc/
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/c3server/hal/gnss/include/
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/c3server/hal/modem/include/
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/c3server/srsdk/pm/inc/
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/c3server/srsdk/param/inc/
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/c3server/srsdk/modem/inc/
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/c3server/sripc/inc/
)

find_library(JSONCPP_LIBRARIES
    NAMES jsoncpp
    PATHS ${PROJECT_SOURCE_DIR}/../../../app-third-lib/lib
    NO_DEFAULT_PATH
)

find_library(MQTT_LIBRARIES
    NAMES paho-mqtt3cs
    PATHS ${PROJECT_SOURCE_DIR}/../../../app-third-lib/lib
    NO_DEFAULT_PATH
)

find_library(PROTOBUF_LIBRARIES
    NAMES protobuf
    PATHS ${PROJECT_SOURCE_DIR}/../../../app-third-lib/lib
    NO_DEFAULT_PATH
)

target_link_libraries(${PROJECT_NAME}
    pthread
    curl

    sqlite3
    glib-2.0

    ${JSONCPP_LIBRARIES}
    ${MQTT_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
)

# 安装目标
install(DIRECTORY inc/public/ DESTINATION include/terminal_management)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

# 设置变量
# set(MONITOR_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/config/monitor_config.json")

# 安装配置文件到指定目录
# install(FILES ${MONITOR_CONFIG} DESTINATION ${CMAKE_INSTALL_PREFIX}/../../oemapp/config RENAME monitor_config.json)