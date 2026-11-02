cmake_minimum_required(VERSION 3.0)
project(modemServer)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../hal/modem/include
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../common_util/inc
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
)

add_executable(modemServer
    src/modem_ant.c
    src/modem_call.c
    src/modem_dms.c
    src/modem_main.c
    src/modem_nw.c
    src/modem_rpc.c
    src/modem_sim.c
    src/modem_sms.c
    src/modem_sms_record_db_api.c
    ../common_util/src/msg_queue.c
)

add_dependencies(modemServer sr-ipc)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "A7805")
target_link_libraries(modemServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    mtel
    sqlite3
)
else()
target_link_libraries(modemServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sqlite3
)
endif()

# 设置可执行文件名称
set_target_properties(modemServer PROPERTIES OUTPUT_NAME "modemServer")

# 安装目标
install(TARGETS modemServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)


