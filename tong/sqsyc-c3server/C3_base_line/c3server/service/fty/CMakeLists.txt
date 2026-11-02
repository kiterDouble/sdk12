cmake_minimum_required(VERSION 3.0)
project(ftyServer)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/net/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/ecall/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/era/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/wifi/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/mcu/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/modem/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/param/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc
    ${PROJECT_SOURCE_DIR}/../../hal/modem/include
)

add_executable(ftyServer
	src/fty_control.c
    src/fty_crt.c
    src/fty_info.c
    src/fty_main.c
)

add_dependencies(ftyServer sr-hal)
add_dependencies(ftyServer sr-ipc)
add_dependencies(ftyServer sr-sdk)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
target_link_libraries(ftyServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    ql_common_api
    ql_peripheral
)
endif()
if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
target_link_libraries(ftyServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
)
endif()
if (${MODEMTYPE} STREQUAL "A7805")
target_link_libraries(ftyServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${FFI_LIBRARY}
    ${GMODULE2_LIBRARY}
    z
    mount
    blkid
    mtel
)
endif()
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(ftyServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    z
    mount
    blkid
)
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
target_link_libraries(ftyServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    z
    mount
    blkid
)
endif()
# 设置可执行文件名称
set_target_properties(ftyServer PROPERTIES OUTPUT_NAME "ftyServer")

# 安装目标
install(TARGETS ftyServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
