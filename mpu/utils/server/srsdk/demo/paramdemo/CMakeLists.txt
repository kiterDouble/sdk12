cmake_minimum_required(VERSION 3.0)
project(paramDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../param/inc
)

add_executable(paramDemo
    param_demo.c
)

# 链接所需的库
target_link_libraries(paramDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(paramDemo PROPERTIES OUTPUT_NAME "paramDemo")

# 安装目标
install(TARGETS paramDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

