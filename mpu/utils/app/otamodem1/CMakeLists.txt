cmake_minimum_required(VERSION 3.0)
project(modem_a)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/cjson
    ${PROJECT_SOURCE_DIR}/inc/http
    ${PROJECT_SOURCE_DIR}/inc/misc
    
)

add_executable(modem_a
	"${PROJECT_SOURCE_DIR}/src/cjson/cJSON.c"
	"${PROJECT_SOURCE_DIR}/src/cjson/cjson_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/http/https_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/http/https.cpp"

       
	"${PROJECT_SOURCE_DIR}/src/http/http_queue.cpp"
	"${PROJECT_SOURCE_DIR}/src/http/http_thread.cpp"

	"${PROJECT_SOURCE_DIR}/src/misc/modem_function.cpp"
	"${PROJECT_SOURCE_DIR}/src/misc/system_interface.cpp" 
	"${PROJECT_SOURCE_DIR}/src/misc/thread_interface.cpp" 
	"${PROJECT_SOURCE_DIR}/src/misc/file_interface.cpp" 
	"${PROJECT_SOURCE_DIR}/src/misc/time_interface.cpp" 
    "${PROJECT_SOURCE_DIR}/src/misc/otamodule_interface.cpp"
    "${PROJECT_SOURCE_DIR}/src/misc/completion_timeout.cpp"
    
	"${PROJECT_SOURCE_DIR}/src/main.cpp"
)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce")
target_link_libraries(modem_a
    pthread
    sqlite3
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    protobuf
    curl
    ${ZLOG_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
)
endif()
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(modem_a
    pthread
    sqlite3
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    curl
    rt
    ${ZLOG_LIBRARY}
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${EVENTCORE_LIBRARY}
    ${EVENTPTHREADS_LIBRARY}
    

)
endif()
# 设置可执行文件名称
set_target_properties(modem_a PROPERTIES OUTPUT_NAME "modem_a")

# 安装目标
install(TARGETS modem_a  DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

