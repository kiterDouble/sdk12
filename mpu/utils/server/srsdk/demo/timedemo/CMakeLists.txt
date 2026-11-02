cmake_minimum_required(VERSION 3.0)
project(timeDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../doip/include
)

add_executable(timeDemo
    time_demo.c
)

# 链接所需的库
target_link_libraries(timeDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(timeDemo PROPERTIES OUTPUT_NAME "timeDemo")

# 安装目标
install(TARGETS timeDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

