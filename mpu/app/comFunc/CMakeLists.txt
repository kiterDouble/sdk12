cmake_minimum_required(VERSION 3.0)
project(sr-comfunc)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
)

add_library(sr-comfunc SHARED
        "${PROJECT_SOURCE_DIR}/src/common_functions.c"
)

target_link_libraries(sr-comfunc 
    ${GLIB2_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
)

# 安装目标
install(TARGETS sr-comfunc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

