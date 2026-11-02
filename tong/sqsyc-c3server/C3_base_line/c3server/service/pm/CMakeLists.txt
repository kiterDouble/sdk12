cmake_minimum_required(VERSION 3.0)
project(pmServer)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/mcu/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc   
    ${PROJECT_SOURCE_DIR}/../../hal/modem/include
    ${PROJECT_SOURCE_DIR}/../../srsdk/property/inc
    ${SDKTARGETSYSROOT}/usr/include/quectel-openlinux-sdk/
)

add_executable(pmServer
    src/cJSON.c
    src/pm_device_control.c
    src/pm_main.c
    src/pm_event.c
    src/pm_timer.c
    src/pm_ipc_rpc.c
    src/pm_modem.c
    src/pm_modem_reboot.c
    src/pm_modem_reboot_config.c
    src/pm_sched.c
    src/pm_io.c
    src/pm_mcu_control.c
)

add_dependencies(pmServer sr-hal)
add_dependencies(pmServer sr-ipc)
add_dependencies(pmServer sr-sdk)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
target_link_libraries(pmServer
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
target_link_libraries(pmServer
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
target_link_libraries(pmServer
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
target_link_libraries(pmServer
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
target_link_libraries(pmServer
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
set_target_properties(pmServer PROPERTIES OUTPUT_NAME "pmServer")

# 安装目标
install(TARGETS pmServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)

# 设置变量
set(CE_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/pm_config_ce.json")
set(EVD_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/pm_config_evd.json")
set(CET_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/pm_config_cet.json")
set(A7805_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/pm_config_A7805.json")
set(AG591_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/pm_config_AG591.json")
set(ZM8201_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/pm_config_ZM8201.json")
# 适配文件
if (${MODEMTYPE} STREQUAL "ag35ce")
    set(CONFIG_FILE ${CE_CONFIG_FILE})
elseif (${MODEMTYPE} STREQUAL "ag35evd")
    set(CONFIG_FILE ${EVD_CONFIG_FILE})
elseif (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
    set(CONFIG_FILE ${CET_CONFIG_FILE})
elseif (${MODEMTYPE} STREQUAL "A7805")
    set(CONFIG_FILE ${A7805_CONFIG_FILE})
elseif (${MODEMTYPE} STREQUAL "AG591")
    set(CONFIG_FILE ${AG591_CONFIG_FILE})
elseif (${MODEMTYPE} STREQUAL "ZM8201")
    set(CONFIG_FILE ${ZM8201_CONFIG_FILE})
else ()
    message(FATAL_ERROR "Unsupported MODEMTYPE")
endif ()

# 安装配置文件到指定目录
# install(FILES ${CONFIG_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/../config RENAME pm_config.json)
# install(FILES ${CONFIG_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/config RENAME pm_config.json)