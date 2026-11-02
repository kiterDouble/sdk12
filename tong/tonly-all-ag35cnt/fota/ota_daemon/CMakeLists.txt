cmake_minimum_required(VERSION 3.0)
project(otaDaemon)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_env.cmake)

include_directories(
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/server-out/include
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/server-out/include/common_inc
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/server-out/include/hal
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/server-out/include/sdk
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/thrid-lib-out/include
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/utils-out/include/utils_inc
)

add_executable(otaDaemon
    src/upgrade_main.c
)

# 链接所需的库
target_link_libraries(otaDaemon
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
        pthread
        sr-hal
        sr-ipc
        sr-sdk
        ${CURL_LIBRARY}
        ${ZLOG_LIBRARY}
        ${GLIB2_LIBRARY}
        ${GIO_LIBRARY}
        ${GOBJECT_LIBRARY}
        ${FFI_LIBRARY}
        ${GMODULE2_LIBRARY}
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
        ${UUID_LIBRARY}
)

# 安装目标
install(TARGETS otaDaemon DESTINATION ${CMAKE_INSTALL_PREFIX}/)

