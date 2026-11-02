cmake_minimum_required(VERSION 3.0)
project(doipUpgradeDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../../doip/include
    ${PROJECT_SOURCE_DIR}/common/inc
    ${PROJECT_SOURCE_DIR}/tester/inc
    ${PROJECT_SOURCE_DIR}/entity/inc
)

add_executable(doipUpgradeTester
    common/src/msg_queue.c
    common/src/algorithm.c
    tester/src/doip_tp.c
    tester/src/ihex_copy.c
    tester/src/ihex_parse.c
    tester/src/ihex_record.c
    tester/src/flash_info.c
    tester/src/tester_main.c
)

add_executable(doipUpgradeEntity
    common/src/msg_queue.c
    common/src/algorithm.c
    entity/src/entity_main.c
)
# 链接所需的库
target_link_libraries(doipUpgradeTester
    sr-sdk
)

target_link_libraries(doipUpgradeEntity
    sr-sdk
)

# 设置可执行文件名称
set_target_properties(doipUpgradeTester PROPERTIES OUTPUT_NAME "doipUpgradeTester")
set_target_properties(doipUpgradeEntity PROPERTIES OUTPUT_NAME "doipUpgradeEntity")
# 安装目标
install(TARGETS doipUpgradeTester DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)
install(TARGETS doipUpgradeEntity DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

