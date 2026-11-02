cmake_minimum_required(VERSION 3.0)
project(hal-modem)

# 参与编译的源码
if (${MODEMTYPE} STREQUAL "A7805")
set(HAL_SOURCE_CODE_PATH
    "${HAL_SOURCE_CODE_PATH}"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_comm.c"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_ant.c"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_call.c"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_dial.c"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_dms.c"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_nw.c"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_pm.c"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_sim.c"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_sms.c"
    "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_upgrade.c"
    PARENT_SCOPE)
elseif(${MODEMTYPE} STREQUAL "ag35cet-openssl3")
    set(HAL_SOURCE_CODE_PATH
        "${HAL_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_ant.c"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_call.c"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_dial.c"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_dms.c"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_nw.c"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_pm.c"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_sim.c"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_sms.c"
        "${PROJECT_SOURCE_DIR}/ag35cet/hal_modem_upgrade.c"
        PARENT_SCOPE)
elseif (${MODEMTYPE} STREQUAL "AG591")
    set(HAL_SOURCE_CODE_PATH
        "${HAL_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_ant.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_call.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_dial.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_dms.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_nw.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_pm.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_sim.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_sms.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_upgrade.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_tz.c"
        PARENT_SCOPE)
else()
    set(HAL_SOURCE_CODE_PATH
        "${HAL_SOURCE_CODE_PATH}"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_ant.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_call.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_dial.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_dms.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_nw.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_pm.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_sim.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_sms.c"
        "${PROJECT_SOURCE_DIR}/${MODEMTYPE}/hal_modem_upgrade.c"
        PARENT_SCOPE)
endif()

if (${MODEMTYPE} STREQUAL "AG591")
set(HAL_INSTALL_HEADER_PATH 
    "${HAL_INSTALL_HEADER_PATH}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_ant.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_call.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_dial.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_dms.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_nw.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_pm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_sim.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_sms.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_upgrade.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_tz.h"
    PARENT_SCOPE)
else()
# 需要install的头文件
set(HAL_INSTALL_HEADER_PATH 
    "${HAL_INSTALL_HEADER_PATH}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_ant.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_call.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_dial.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_dms.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_nw.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_pm.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_sim.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_sms.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hal_modem_upgrade.h"
    PARENT_SCOPE)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")

    add_executable(ag35cet_upgrade_sync
        ${PROJECT_SOURCE_DIR}/ag35cet/upgrade_sync.c
    )

    # 链接所需的库
    target_link_libraries(ag35cet_upgrade_sync
        ${ZLOG_LIBRARY}
        ql_lib_absys
        ql_lib_fota
        ql_lib_ipc
        ql_sys_log
        ql_lib_utils
    )

    # 设置可执行文件名称
    set_target_properties(ag35cet_upgrade_sync PROPERTIES OUTPUT_NAME "ag35cet_upgrade_sync")

    # 安装目标
    install(TARGETS ag35cet_upgrade_sync DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
endif()


if (${MODEMTYPE} STREQUAL "ag519")

    add_executable(ag519_upgrade_sync
        ${PROJECT_SOURCE_DIR}/ag519/upgrade_sync.c
    )

    # 链接所需的库
    target_link_libraries(ag519_upgrade_sync
        ${ZLOG_LIBRARY}
        absys
        fotaapi
    )

    # 设置可执行文件名称
    set_target_properties(ag519_upgrade_sync PROPERTIES OUTPUT_NAME "ag519_upgrade_sync")

    # 安装目标
    install(TARGETS ag519_upgrade_sync DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
endif()

if (${MODEMTYPE} STREQUAL "AG591")

    include_directories(
        ${SDKTARGETSYSROOT}/usr/include/ql_lib_utils
    )

    add_executable(ag591_upgrade_sync
        ${PROJECT_SOURCE_DIR}/AG591/upgrade_sync.c
    )

    # 链接所需的库
    target_link_libraries(ag591_upgrade_sync
        ${ZLOG_LIBRARY}
        absys
        fotaapi
    )

    # 设置可执行文件名称
    set_target_properties(ag591_upgrade_sync PROPERTIES OUTPUT_NAME "ag591_upgrade_sync")

    # 安装目标
    install(TARGETS ag591_upgrade_sync DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
endif()