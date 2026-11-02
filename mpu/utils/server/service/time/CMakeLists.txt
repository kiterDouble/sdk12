cmake_minimum_required(VERSION 3.0)
project(timeServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++11 -D_GNU_SOURCE -Wno-deprecated -Wno-psabi")

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/mcu/inc/
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
)

add_executable(timeServer
        "${PROJECT_SOURCE_DIR}/src/time_gnss.c"
        "${PROJECT_SOURCE_DIR}/src/time_ntp.c"
        "${PROJECT_SOURCE_DIR}/src/time_main.c"
)

add_dependencies(timeServer sr-ipc)
add_dependencies(timeServer sr-sdk)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
target_link_libraries(timeServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
)
endif()
if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
target_link_libraries(timeServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    rt
)
endif()
if (${MODEMTYPE} STREQUAL "A7805")
target_link_libraries(timeServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    rt
    ${FFI_LIBRARY}
    ${GMODULE2_LIBRARY}
    z
    mount
    blkid
    mtel
)
endif()
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(timeServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    z
    mount
    blkid
)
endif()
if (${MODEMTYPE} STREQUAL "ZM8201")
target_link_libraries(timeServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    z
    mount
    blkid
)
endif()

# 设置可执行文件名称
set_target_properties(timeServer PROPERTIES OUTPUT_NAME "timeServer")

# 安装目标
install(TARGETS timeServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/inc/time_status.h  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/)
