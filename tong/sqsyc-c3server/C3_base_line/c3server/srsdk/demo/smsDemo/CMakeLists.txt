cmake_minimum_required(VERSION 3.0)
project(smsDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../modem/inc
    ${PROJECT_SOURCE_DIR}/../../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../../common_inc
    ${PROJECT_SOURCE_DIR}/../../pm/inc
    ${PROJECT_SOURCE_DIR}/../../sdkinit/inc
    ${PROJECT_SOURCE_DIR}/../../../hal/modem/include
)

add_executable(smsDemo
    sms_demo.c
)

add_dependencies(smsDemo sr-sdk)
add_dependencies(smsDemo sr-hal)
add_dependencies(smsDemo sr-ipc)

target_link_libraries(smsDemo
    sr-sdk
    sr-hal
    sr-ipc
)

set_target_properties(smsDemo PROPERTIES OUTPUT_NAME "smsDemo")

install(TARGETS smsDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)
