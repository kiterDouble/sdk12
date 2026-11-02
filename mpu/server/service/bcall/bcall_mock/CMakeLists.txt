cmake_minimum_required(VERSION 3.0)
project(bcallMock)

file(GLOB ECALL_INC "${PROJECT_SOURCE_DIR}/bcall_manager/inc" 
              )
              
include_directories(
    ${ECALL_INC}
)

file(GLOB ECALL_SRC "${PROJECT_SOURCE_DIR}/src/bcall_mock.c" 
              )
              
add_executable(bcallMock ${ECALL_SRC})

# 链接所需的库
target_link_libraries(bcallMock
    ${ZLOG_LIBRARY}
)

# 设置可执行文件名称
set_target_properties(bcallMock PROPERTIES OUTPUT_NAME "bcallMock")

# 安装目标
install(TARGETS bcallMock DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

