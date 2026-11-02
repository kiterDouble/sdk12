cmake_minimum_required(VERSION 3.0)
project(cpuMonitorServer)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc
)

add_executable(${PROJECT_NAME}
    src/main.c
    src/cpu_info_monitor.c
    src/cpu_info_proc.c
    src/cpu_info_statistics.c
    src/cpu_info_usage.c
    src/cpu_pm.c
)

add_dependencies(${PROJECT_NAME} sr-ipc)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
target_link_libraries(${PROJECT_NAME}
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
target_link_libraries(${PROJECT_NAME}
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    rt
    pthread
)
endif()

if (${MODEMTYPE} STREQUAL "A7805")
target_link_libraries(${PROJECT_NAME}
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${FFI_LIBRARY}
    ${GMODULE2_LIBRARY}
    rt
    pthread
)
endif()

if (${MODEMTYPE} STREQUAL "ag519" OR ${MODEMTYPE} STREQUAL "AG591" OR ${MODEMTYPE} STREQUAL "ZM8201")
target_link_libraries(${PROJECT_NAME}
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
)
endif()

# 设置可执行文件名称
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "cpuMonitorServer")

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
