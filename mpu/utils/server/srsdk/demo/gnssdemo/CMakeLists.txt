cmake_minimum_required(VERSION 3.0)
project(gnssDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../gnss/inc
)

add_executable(gnssDemo
    gnss_demo.c
)

# 链接所需的库
target_link_libraries(gnssDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(gnssDemo PROPERTIES OUTPUT_NAME "gnssDemo")

# 安装目标
install(TARGETS gnssDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

