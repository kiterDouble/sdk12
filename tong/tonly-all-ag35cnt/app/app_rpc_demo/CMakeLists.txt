cmake_minimum_required(VERSION 3.0)
project(app_rpc_demo)


# 公共包含目录
include_directories(
    ../app_util/inc
    ../app_rpc/inc
    src/util
)

set(COMMON_LINK_DIRECTORIES
    /oemapp/lib
    /oemapp/third_lib
)


# ========== rpc_demo_digital_key ==========
add_executable(rpc_demo_digital_key src/digital_key/rpc_demo_digital_key.c)
target_compile_options(rpc_demo_digital_key PRIVATE -Wno-error)
target_link_libraries(rpc_demo_digital_key PUBLIC app app_rpc)
target_link_directories(rpc_demo_digital_key PRIVATE ${COMMON_LINK_DIRECTORIES})
install(TARGETS rpc_demo_digital_key DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)


