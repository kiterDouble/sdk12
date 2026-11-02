cmake_minimum_required(VERSION 3.0)
project(dtcApp)

include_directories(
    ../app_rpc/inc
    inc    
    ${GLIB_INCLUDE_DIRS}  # GLib头文件路径
)

# set(CMAKE_CXX_STANDARD 11)

add_executable(${PROJECT_NAME}
        src/didRegistry.c
        src/dbDriver.c
        src/didDefaults.c
        src/udsGateway.c
        src/udsCallbackEntry.c
        # src/logRecall.c
        # src/logRecallGmClient.c
        src/dbBackupService.c
        src/fty.c
        src/mdio.c
        main.c
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC app
    app_rpc
)

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/apps)

# 如需手动开启 GNU 扩展：
# target_compile_definitions(dtcApp PRIVATE _GNU_SOURCE)
