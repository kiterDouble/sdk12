cmake_minimum_required(VERSION 3.0)
project(doipserverDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../doip/include
)

add_executable(doipserverDemo
    server.c
)

# 链接所需的库
target_link_libraries(doipserverDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(doipserverDemo PROPERTIES OUTPUT_NAME "doipserverDemo")

# 安装目标
install(TARGETS doipserverDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

