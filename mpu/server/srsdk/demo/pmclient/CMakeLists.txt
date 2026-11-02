cmake_minimum_required(VERSION 3.0)
project(pmclientDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../pm/inc
)

add_executable(pmclientDemo
    pm_api_demo.c
)

# 链接所需的库
target_link_libraries(pmclientDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(pmclientDemo PROPERTIES OUTPUT_NAME "pmclientDemo")

# 安装目标
install(TARGETS pmclientDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

