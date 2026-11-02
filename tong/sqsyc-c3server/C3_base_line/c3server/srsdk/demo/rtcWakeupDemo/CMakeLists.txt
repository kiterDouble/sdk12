cmake_minimum_required(VERSION 3.0)
project(rtcWakeupDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../pm/inc
    ${PROJECT_SOURCE_DIR}/../../sdkinit/inc
    ${PROJECT_SOURCE_DIR}/../../../hal/modem/include
)

add_executable(rtcWakeupDemo
    rtc_wakeup_demo.c
)

add_dependencies(rtcWakeupDemo sr-sdk)
add_dependencies(rtcWakeupDemo sr-hal)

# 同时链接 sr-sdk（公开 PM API）与 sr-hal（HalModemPmWakeLockLockedEx/UnlockedEx）
target_link_libraries(rtcWakeupDemo
    sr-sdk
    sr-hal
)

set_target_properties(rtcWakeupDemo PROPERTIES OUTPUT_NAME "rtcWakeupDemo")

install(TARGETS rtcWakeupDemo DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)
