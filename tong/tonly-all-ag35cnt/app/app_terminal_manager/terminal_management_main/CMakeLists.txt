cmake_minimum_required(VERSION 3.0)
project(terminalManager)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(terminalManagerApp src/terminal_management_main.cpp)

target_include_directories(terminalManagerApp PRIVATE
    ${PROJECT_SOURCE_DIR}/inc/
    ${SDKTARGETSYSROOT}/usr/include/glib-2.0
    ${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include
    ${PROJECT_SOURCE_DIR}/../../lib/include
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/c3server/sripc/inc/
)

find_library(PROTOBUF_LIBRARIES
    NAMES protobuf
    PATHS ${PROJECT_SOURCE_DIR}/../../../app-third-lib/lib
    NO_DEFAULT_PATH
)

target_link_libraries(terminalManagerApp
    terminal_management

    ${PROTOBUF_LIBRARIES}

    # === 顶层库 ===
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/C3BaseLineOut/server-out/lib/libsr-sdk.so
    # === libsr-sdk.so 的直接依赖===
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/C3BaseLineOut/thrid-lib-out/lib/libzlog.so
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/C3BaseLineOut/server-out/lib/libsr-ipc.so
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/C3BaseLineOut/server-out/lib/libsr-hal.so
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/C3BaseLineOut/thrid-lib-out/lib/libevent_core.so
    ${PROJECT_SOURCE_DIR}/../../../C3_base_line/C3BaseLineOut/thrid-lib-out/lib/libevent_pthreads.so
    # Quectel SDK 库
    ${SDKTARGETSYSROOT}/usr/lib/libql_sdk.so

    glib-2.0
    rt
    pthread
)

set_target_properties(terminalManagerApp PROPERTIES OUTPUT_NAME "terminal_manager")
install(TARGETS terminalManagerApp DESTINATION ${CMAKE_INSTALL_PREFIX}/apps/)