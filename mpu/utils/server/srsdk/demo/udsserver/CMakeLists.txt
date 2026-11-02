cmake_minimum_required(VERSION 3.0)
project(udssrvDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../doip/include
)

add_executable(udssrvDemo
    uds_server_demo.c
)

# 链接所需的库
target_link_libraries(udssrvDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(udssrvDemo PROPERTIES OUTPUT_NAME "udssrvDemo")

# 安装目标
install(TARGETS udssrvDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

