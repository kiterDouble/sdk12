cmake_minimum_required(VERSION 3.0)
project(rpcClientDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../common/
)

add_executable(rpcClientDemo
    rpcClientDemo.c
)

# 链接所需的库
target_link_libraries(rpcClientDemo
    sr-ipc
)

# 设置可执行文件名称
set_target_properties(rpcClientDemo PROPERTIES OUTPUT_NAME "rpcClientDemo")

# 安装目标
install(TARGETS rpcClientDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

