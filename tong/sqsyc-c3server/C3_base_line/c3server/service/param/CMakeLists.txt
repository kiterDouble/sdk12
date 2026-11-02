cmake_minimum_required(VERSION 3.0)
project(paramServer)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/param/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/mcu/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/modem/inc
)

add_executable(paramServer
    src/param_main.c
    src/param_database.c
    src/param_rpc_handle.c
    src/param_sync.c
    src/param_backup.c
    src/param_encrypt.c
    src/param_util.c
)

add_dependencies(paramServer sr-ipc)
add_dependencies(paramServer sr-sdk)

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
target_link_libraries(paramServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
    sqlite3
    dl
    crypto
    ssl
)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
target_link_libraries(paramServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${FFI_LIBRARY}
    ${GMODULE2_LIBRARY}
    rt
    pthread
    sqlite3
    dl
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
)
endif()

if (${MODEMTYPE} STREQUAL "A7805")
target_link_libraries(paramServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${FFI_LIBRARY}
    ${GMODULE2_LIBRARY}
    rt
    pthread
    ${SQLITE3_LIBRARY}
    z
    mount
    blkid
    dl
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    mtel
)
endif()
if (${MODEMTYPE} STREQUAL "ag519")
target_link_libraries(paramServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
    sqlite3
    dl
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
)
endif()
if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(paramServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
    sqlite3
    dl
    crypto
    ssl
)
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
target_link_libraries(paramServer
    ${ZLOG_LIBRARY}
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
    sqlite3
    dl
    crypto
    ssl
)
endif()

# 设置可执行文件名称
set_target_properties(paramServer PROPERTIES OUTPUT_NAME "paramServer")

# 安装目标
install(TARGETS paramServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
set(DEFAULT_DB_FILE "${CMAKE_CURRENT_SOURCE_DIR}/defaultDB/paramsDb")
install(FILES ${DEFAULT_DB_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/parameters)

