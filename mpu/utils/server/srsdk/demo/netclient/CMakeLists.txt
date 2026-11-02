cmake_minimum_required(VERSION 3.0)
project(netclientDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../net/inc
)

add_executable(netclientDemo
    net_api_demo.c
)

# 链接所需的库
target_link_libraries(netclientDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(netclientDemo PROPERTIES OUTPUT_NAME "netclientDemo")

# 安装目标
install(TARGETS netclientDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

