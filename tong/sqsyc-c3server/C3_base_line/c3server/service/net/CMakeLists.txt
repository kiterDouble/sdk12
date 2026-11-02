cmake_minimum_required(VERSION 3.0)
project(netServer)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/inc/utils/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/net/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/modem/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/param/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/mcu/inc
    ${PROJECT_SOURCE_DIR}/../../hal/modem/include
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
)

add_executable(netServer
	src/net_main.c
    src/net_rpc.c
    src/net_service.c
    src/net_service_modem_info.c
    src/net_service_pm_state.c
    src/net_service_dns_monitor.c
    src/net_service_mcu_sync.c
    src/net_service_state_monitor.c
    src/net_dial_record_db_api.c
    src/net_host_resolve.c
    src/net_config.c
    src/net_route_cache.c
    src/net_timer.c
    src/net_backoff.c
    src/net_bewinner_sim_sw_task.c
    src/net_route_domain.c
    src/utils/net_interface_utils.c
    src/utils/net_route_utils.c
    src/utils/net_shell_utils.c
    src/utils/common_utils.c
)

add_dependencies(netServer sr-hal)
add_dependencies(netServer sr-ipc)
add_dependencies(netServer sr-sdk)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
target_link_libraries(netServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    ql_common_api
    ql_peripheral
    sqlite3
)
endif()
if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
target_link_libraries(netServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    sqlite3
)
endif()
if (${MODEMTYPE} STREQUAL "A7805")
target_link_libraries(netServer
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
    sqlite3
)
endif()
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(netServer
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
    sqlite3
)
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
target_link_libraries(netServer
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
    sqlite3
)
endif()

# 设置可执行文件名称
set_target_properties(netServer PROPERTIES OUTPUT_NAME "netServer")

# 安装目标
install(TARGETS netServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/script/vlan_set.sh  DESTINATION ${CMAKE_INSTALL_PREFIX}/script/)
