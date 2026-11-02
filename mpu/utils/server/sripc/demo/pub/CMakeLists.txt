cmake_minimum_required(VERSION 3.0)
project(pubdemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../common/
)

add_executable(pubdemo
    pub_demo.c
)

# 链接所需的库
target_link_libraries(pubdemo
    ${GLIB2_LIBRARY}
    sr-ipc
)

# 设置可执行文件名称
set_target_properties(pubdemo PROPERTIES OUTPUT_NAME "pubdemo")

# 安装目标
install(TARGETS pubdemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

