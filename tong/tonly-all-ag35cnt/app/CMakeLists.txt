cmake_minimum_required(VERSION 3.0)
project(tbox_app)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_env.cmake)

add_library(app INTERFACE)


include_directories(
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/server-out/include
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/server-out/include/common_inc
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/server-out/include/hal
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/server-out/include/sdk
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/thrid-lib-out/include
    ${PROJECT_SOURCE_DIR}/../C3_base_line/C3BaseLineOut/utils-out/include/utils_inc
)

# 链接所需的库
target_link_libraries(app INTERFACE
        sr-hal
        sr-ipc
        sr-sdk
        ${ZLOG_LIBRARY}
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
        glib-2.0
        gobject-2.0
        gmodule-2.0
        gthread-2.0
        pthread
        sqlite3
        m
)

add_subdirectory(app_rpc)
add_subdirectory(app_rpc_demo)
add_subdirectory(openssl_sirun_lib)
add_subdirectory(ota_master_lib)
add_subdirectory(dtc)
add_subdirectory(imokdemo)
add_subdirectory(app_tsp)
add_subdirectory(demo/otamasterDemo)
add_subdirectory(app_terminal_manager)
