cmake_minimum_required(VERSION 3.0)
project(eraMock)

file(GLOB ECALL_INC "${PROJECT_SOURCE_DIR}/era_manager/inc" 
              )
              
include_directories(
    ${ECALL_INC}
)

file(GLOB ECALL_SRC "${PROJECT_SOURCE_DIR}/src/era_mock.c" 
              )
file(GLOB ECALL_TEST_SRC "${PROJECT_SOURCE_DIR}/../era_comm/src/era_timer.c"
                         "${PROJECT_SOURCE_DIR}/src/era_test.c"
                        )

#message(STATUS "---source path:${ECALL_TEST_SRC}")

add_executable(eraMock ${ECALL_SRC})
add_executable(eraTest ${ECALL_TEST_SRC})

# 链接所需的库
target_link_libraries(eraMock
    ${ZLOG_LIBRARY}
)
target_link_libraries(eraTest
    ${ZLOG_LIBRARY}
)

# 设置可执行文件名称
set_target_properties(eraMock PROPERTIES OUTPUT_NAME "eraMock")
set_target_properties(eraTest PROPERTIES OUTPUT_NAME "eraTest")

# 安装目标
install(TARGETS eraMock DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)
install(TARGETS eraTest DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

