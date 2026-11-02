cmake_minimum_required(VERSION 3.0)
project(opensslEngineDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../opensslengine/inc
)

add_executable(opensslEngineDemo
    opensslEngineDemo.c
)

# 链接所需的库
target_link_libraries(opensslEngineDemo
    sr-sdk
)

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
    target_compile_options(opensslEngineDemo PRIVATE -Wno-deprecated-declarations)
endif()

# 设置可执行文件名称
set_target_properties(opensslEngineDemo PROPERTIES OUTPUT_NAME "opensslEngineDemo")

# 安装目标
install(TARGETS opensslEngineDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

