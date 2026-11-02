cmake_minimum_required(VERSION 3.0)
project(ecallMock)

file(GLOB ECALL_INC "${PROJECT_SOURCE_DIR}/ecall_manager/inc" 
              )
              
include_directories(
    ${ECALL_INC}
)

file(GLOB ECALL_SRC "${PROJECT_SOURCE_DIR}/src/ecall_mock.c" 
              )
              
add_executable(ecallMock ${ECALL_SRC})

# 链接所需的库
target_link_libraries(ecallMock
    ${ZLOG_LIBRARY}
)

# 设置可执行文件名称
set_target_properties(ecallMock PROPERTIES OUTPUT_NAME "ecallMock")

# 安装目标
install(TARGETS ecallMock DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

