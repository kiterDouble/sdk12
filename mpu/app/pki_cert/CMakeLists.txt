cmake_minimum_required(VERSION 3.0)
project(pki_cert)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/inc/cjson
    ${PROJECT_SOURCE_DIR}/inc/https
    ${PROJECT_SOURCE_DIR}/inc/misc
    ${PROJECT_SOURCE_DIR}/inc/openssl
    
)

# set(LIB_SOURCES
#     "${PROJECT_SOURCE_DIR}/src/cjson/cJSON.c"
    
# 	"${PROJECT_SOURCE_DIR}/src/cjson/cjson_interface.cpp"
# 	"${PROJECT_SOURCE_DIR}/src/https/https_interface.cpp"
# 	"${PROJECT_SOURCE_DIR}/src/https/https.cpp"
#     "${PROJECT_SOURCE_DIR}/src/misc/system_interface.cpp"
#     "${PROJECT_SOURCE_DIR}/src/misc/file_interface.cpp"
#     "${PROJECT_SOURCE_DIR}/src/misc/thread_interface.cpp"
#     "${PROJECT_SOURCE_DIR}/src/openssl/certificate_interface.cpp"
# 	"${PROJECT_SOURCE_DIR}/src/pki_sdk.cpp"
# )

# add_library(pki_sdk SHARED ${LIB_SOURCES})
add_executable(pki_cert
	"${PROJECT_SOURCE_DIR}/src/cjson/cJSON.c"
	"${PROJECT_SOURCE_DIR}/src/cjson/cjson_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/https/https_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/https/https.cpp"
    "${PROJECT_SOURCE_DIR}/src/misc/file_interface.cpp"
    "${PROJECT_SOURCE_DIR}/src/misc/system_interface.cpp"
    "${PROJECT_SOURCE_DIR}/src/misc/thread_interface.cpp"
    "${PROJECT_SOURCE_DIR}/src/openssl/certificate_interface.cpp"
	"${PROJECT_SOURCE_DIR}/src/pki_sdk.cpp"
)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce")
target_link_libraries(pki_sdk
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
target_link_libraries(pki_cert
    pthread
    sqlite3
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
   # sr-comfunc
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
set_target_properties(pki_cert PROPERTIES OUTPUT_NAME "pki_cert")

# 安装目标
install(TARGETS pki_cert  DESTINATION ${CMAKE_INSTALL_PREFIX}/app/)

