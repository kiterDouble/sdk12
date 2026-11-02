cmake_minimum_required(VERSION 3.0)
project(rpcServerDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../common/
)

add_executable(rpcServerDemo
    rpcServerDemo.c
)

# 链接所需的库
target_link_libraries(rpcServerDemo
    sr-ipc
)

# 设置可执行文件名称
set_target_properties(rpcServerDemo PROPERTIES OUTPUT_NAME "rpcServerDemo")

# 安装目标
install(TARGETS rpcServerDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

