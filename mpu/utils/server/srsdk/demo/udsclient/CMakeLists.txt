cmake_minimum_required(VERSION 3.0)
project(udscltDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../doip/include
)

add_executable(udscltDemo
    uds_client_demo.c
)

# 链接所需的库
target_link_libraries(udscltDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(udscltDemo PROPERTIES OUTPUT_NAME "udscltDemo")

# 安装目标
install(TARGETS udscltDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

