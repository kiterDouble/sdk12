cmake_minimum_required(VERSION 3.0)
project(imokDemo)

add_executable(imokDemo
    imok_demo.c
)

target_link_libraries(imokDemo 
    app 
)

# 设置可执行文件名称
set_target_properties(imokDemo PROPERTIES OUTPUT_NAME "imok")

# 安装目标
install(TARGETS imokDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/apps/)

