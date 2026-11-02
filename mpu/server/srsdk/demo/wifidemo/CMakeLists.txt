cmake_minimum_required(VERSION 3.0)
project(wifidemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../doip/include
)

add_executable(wifi_ap_mode
    wifi_ap.c
)

# 链接所需的库
target_link_libraries(wifi_ap_mode
    sr-sdk
)

add_executable(wifi_sta_mode
    wifi_sta.c
)

# 链接所需的库
target_link_libraries(wifi_sta_mode
    sr-sdk
)


add_executable(wifi_ap_ap_mode
    wifi_ap_ap.c
)

# 链接所需的库
target_link_libraries(wifi_ap_ap_mode
    sr-sdk
)


add_executable(wifi_ap_sta_mode
    wifi_ap_sta.c
)

# 链接所需的库
target_link_libraries(wifi_ap_sta_mode
    sr-sdk
)


# 设置可执行文件名称
set_target_properties(wifi_ap_mode PROPERTIES OUTPUT_NAME "wifi_ap_mode")

# 安装目标
install(TARGETS wifi_ap_mode DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

# 设置可执行文件名称
set_target_properties(wifi_sta_mode PROPERTIES OUTPUT_NAME "wifi_sta_mode")

# 安装目标
install(TARGETS wifi_sta_mode DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

# 设置可执行文件名称
set_target_properties(wifi_ap_ap_mode PROPERTIES OUTPUT_NAME "wifi_ap_ap_mode")

# 安装目标
install(TARGETS wifi_ap_ap_mode DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)


# 设置可执行文件名称
set_target_properties(wifi_ap_sta_mode PROPERTIES OUTPUT_NAME "wifi_ap_sta_mode")

# 安装目标
install(TARGETS wifi_ap_sta_mode DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)