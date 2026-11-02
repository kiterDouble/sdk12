cmake_minimum_required(VERSION 3.0)
project(otaMasterDemo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../ota_master_lib/inc/private/
    ${PROJECT_SOURCE_DIR}/../../ota_master_lib/inc/public/
)

add_executable(${PROJECT_NAME}
    src/FunctionTest.cpp
)

add_executable(otaMasterUnitTest
    src/ApiUnitTest.cpp
)

# 链接所需的库（app INTERFACE库包含 sr-ipc, sr-hal, sr-sdk, glib 等平台依赖）
target_link_libraries(${PROJECT_NAME} ota_master app ${JSONCPP_LIBRARIES})
target_link_libraries(otaMasterUnitTest ota_master app ${JSONCPP_LIBRARIES})

# 设置可执行文件名称
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "otaMasterDemo")
set_target_properties(otaMasterUnitTest PROPERTIES OUTPUT_NAME "otaMasterUnitTest")

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)
install(TARGETS otaMasterUnitTest DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)