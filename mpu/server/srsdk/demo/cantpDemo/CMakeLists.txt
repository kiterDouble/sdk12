cmake_minimum_required(VERSION 3.0)
project(cantpDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../docan/inc
)

add_executable(cantpDemo
    cantp_demo.c
)

# 链接所需的库
target_link_libraries(cantpDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(cantpDemo PROPERTIES OUTPUT_NAME "cantpDemo")

# 安装目标
install(TARGETS cantpDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

