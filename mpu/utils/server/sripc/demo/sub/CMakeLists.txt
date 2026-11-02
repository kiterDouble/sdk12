cmake_minimum_required(VERSION 3.0)
project(subdemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../common/
)

add_executable(subdemo
    sub_demo.c
)

# 链接所需的库
target_link_libraries(subdemo
    sr-ipc
)

# 设置可执行文件名称
set_target_properties(subdemo PROPERTIES OUTPUT_NAME "subdemo")

# 安装目标
install(TARGETS subdemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

