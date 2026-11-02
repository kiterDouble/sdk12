cmake_minimum_required(VERSION 3.0)
project(kl30ValtageDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../docan/inc
)

add_executable(kl30ValtageDemo
    kl30Valtage_demo.c
)

# 链接所需的库
target_link_libraries(kl30ValtageDemo
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(kl30ValtageDemo PROPERTIES OUTPUT_NAME "kl30VolDemo")

# 安装目标
install(TARGETS kl30ValtageDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

