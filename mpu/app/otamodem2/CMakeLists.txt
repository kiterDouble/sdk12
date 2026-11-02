cmake_minimum_required(VERSION 3.0)
project(modem_b)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../mcuInterface/inc
)

add_executable(modem_b
    
    ${PROJECT_SOURCE_DIR}/src/file_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/system_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/log_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/md5_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/mtd_interface.cpp
    ${PROJECT_SOURCE_DIR}/src/upgrade_interface.cpp
    ${PROJECT_SOURCE_DIR}/../mcuInterface/mcuinte.c
    ${PROJECT_SOURCE_DIR}/../mcuInterface/mcuinterbase.c
    ${PROJECT_SOURCE_DIR}/src/otamodem2.cpp
)


target_link_libraries(modem_b
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
set_target_properties(modem_b PROPERTIES OUTPUT_NAME "modem_b")

# 安装目标
install(TARGETS modem_b DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

