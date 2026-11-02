cmake_minimum_required(VERSION 3.0)
project(modem_a)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../mcuInterface/inc
    
)

add_executable(modem_a
    "${PROJECT_SOURCE_DIR}/../mcuInterface/mcuinte.c"
    "${PROJECT_SOURCE_DIR}/../mcuInterface/mcuinterbase.c"
	"${PROJECT_SOURCE_DIR}/src/otamodem1.cpp"
	"${PROJECT_SOURCE_DIR}/src/file_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/log_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/mtd_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/system_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/upgrade_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/md5_interface.cpp"
)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce")
target_link_libraries(modem_a
    pthread
    sqlite3
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    protobuf
    curl
    ${ZLOG_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
)
endif()
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(modem_a
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
endif()
# 设置可执行文件名称
set_target_properties(modem_a PROPERTIES OUTPUT_NAME "modem_a")

# 安装目标
install(TARGETS modem_a  DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)
