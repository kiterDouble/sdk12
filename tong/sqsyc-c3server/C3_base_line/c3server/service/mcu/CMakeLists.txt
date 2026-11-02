cmake_minimum_required(VERSION 3.0)
project(mcuServer)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
)
add_executable(mcuServer
    src/mcu_bus.c
    src/mcu_recv.c
    src/mcu_send.c
    src/ring_buffer.c
    src/gpio_api.c
    src/spi_api.c
    src/uart_api.c
    src/mcu_main.c
    src/mcu_can_filter.c
)

add_dependencies(mcuServer sr-ipc)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
    target_link_libraries(mcuServer
        ${ZLOG_LIBRARY}
        ${UTILS_LIBRARY}
        sr-ipc
    )
endif()
if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
    target_link_libraries(mcuServer
        ${ZLOG_LIBRARY}
        ${UTILS_LIBRARY}
        ${CRYPTO_LIBRARY}
        sr-ipc
    )
endif()
if (${MODEMTYPE} STREQUAL "A7805")
    target_link_libraries(mcuServer
        ${ZLOG_LIBRARY}
        ${UTILS_LIBRARY}
        ${CRYPTO_LIBRARY}
        sr-ipc
        mtel
        prop2uci
        uci
        log
        ubox
    )
endif()
if (${MODEMTYPE} STREQUAL "AG591")
    target_link_libraries(mcuServer
        ${ZLOG_LIBRARY}
        ${UTILS_LIBRARY}
        ${CRYPTO_LIBRARY}
        sr-ipc
    )
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
    target_link_libraries(mcuServer
        ${ZLOG_LIBRARY}
        ${UTILS_LIBRARY}
        ${CRYPTO_LIBRARY}
        pthread
        sr-ipc
    )
endif()

# 设置可执行文件名称
set_target_properties(mcuServer PROPERTIES OUTPUT_NAME "mcuServer")

# 安装目标
install(TARGETS mcuServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
