cmake_minimum_required(VERSION 3.0)
project(ethernet_upgrade)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../mcuInterface/inc
    ${PROJECT_SOURCE_DIR}/../server/server-out/include/  # 添加服务器SDK头文件路径
    ${PROJECT_SOURCE_DIR}/../server/srsdk/ota/inc/       # 添加OTA头文件路径
)

add_executable(ethernet_upgrade
    #parm.cpp
    #ok_port_31036.cpp  
    ${PROJECT_SOURCE_DIR}/src/system_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/md5_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/upgrade_interface.cpp
    ${PROJECT_SOURCE_DIR}/../mcuInterface/mcuinte.c
    ${PROJECT_SOURCE_DIR}/../mcuInterface/mcuinterbase.c
    ${PROJECT_SOURCE_DIR}/src/ethernet_upgrade.cpp
    ${PROJECT_SOURCE_DIR}/src/upgrade_socket.cpp
    ${PROJECT_SOURCE_DIR}/src/upgrade_main.cpp
)

target_link_libraries(ethernet_upgrade
    pthread
    sqlite3
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    curl
    rt
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
)

# 设置可执行文件名称
set_target_properties(ethernet_upgrade PROPERTIES OUTPUT_NAME "ethernet_upgrade")

# 安装目标
install(TARGETS ethernet_upgrade DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)