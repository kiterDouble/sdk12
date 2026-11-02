cmake_minimum_required(VERSION 3.0)
project(gnssServer)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../hal/gnss/include
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/gnss/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/mcu/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
)

add_executable(gnssServer
    src/gnss_main.c
    src/gnss_process.c
)

add_dependencies(gnssServer sr-hal)
add_dependencies(gnssServer sr-ipc)
add_dependencies(gnssServer sr-sdk)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd" OR ${MODEMTYPE} STREQUAL "ag519")
    target_link_libraries(gnssServer
        ${ZLOG_LIBRARY}
        sr-hal
        sr-ipc
        sr-sdk
        glib-2.0
        gio-2.0
        gobject-2.0
    )
endif()

if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
    target_link_libraries(gnssServer
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
    target_link_libraries(gnssServer
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
    target_link_libraries(gnssServer
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
    target_link_libraries(gnssServer
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
set_target_properties(gnssServer PROPERTIES OUTPUT_NAME "gnssServer")

# 安装目标
install(TARGETS gnssServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)

