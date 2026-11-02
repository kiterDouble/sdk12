cmake_minimum_required(VERSION 3.0)
project(doipclientDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../doip/include
)

add_executable(doipclientDemo
    client.c
)

# 链接所需的库
target_link_libraries(doipclientDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(doipclientDemo PROPERTIES OUTPUT_NAME "doipclientDemo")

# 安装目标
install(TARGETS doipclientDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

