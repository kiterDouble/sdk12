cmake_minimum_required(VERSION 3.0)
project(logPackServer)

# 动态链接 libarchive
find_library(ARCHIVE_LIBRARY archive
    ${PROJECT_SOURCE_DIR}/../../../C3BaseLineOut/thrid-lib-out/lib/
)
find_library(ZLIB_LIBRARY z
    ${SDKTARGETSYSROOT}/usr/lib/
    ${PROJECT_SOURCE_DIR}/../../../C3BaseLineOut/thrid-lib-out/lib/
)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc/
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/param/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc
)

add_executable(logPackServer
    src/log_pack_main.c
    src/log_pack.c
    src/log_pack_cfg.c
    src/log_pack_compress_and_crypto.c
    src/log_pack_crypto_interface.c
    src/log_pack_crypto_des3_interface.c
    src/log_pack_crypto_aes128_interface.c
    src/log_pack_util.c
    src/log_pm.c
)

add_dependencies(logPackServer sr-hal)
add_dependencies(logPackServer sr-ipc)
add_dependencies(logPackServer sr-sdk)

if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
target_link_libraries(logPackServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
    dl
    crypto
    ssl
    ${ARCHIVE_LIBRARY}
    ${ZLIB_LIBRARY}
)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35eut")
target_link_libraries(logPackServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    rt
    pthread
    dl
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    ${ARCHIVE_LIBRARY}
    ${ZLIB_LIBRARY}
)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
target_link_libraries(logPackServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    rt
    pthread
    dl
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    ${ARCHIVE_LIBRARY}
    ${ZLIB_LIBRARY}
)
target_compile_options(logPackServer PRIVATE -Wno-deprecated-declarations)
endif()

if (${MODEMTYPE} STREQUAL "A7805")
target_link_libraries(logPackServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    ${GLIB2_LIBRARY}
    ${GIO_LIBRARY}
    ${GOBJECT_LIBRARY}
    ${FFI_LIBRARY}
    ${GMODULE2_LIBRARY}
    rt
    pthread
    dl
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    ${ARCHIVE_LIBRARY}
    ${ZLIB_LIBRARY}
    mtel
)
endif()

if (${MODEMTYPE} STREQUAL "ag519")
target_link_libraries(logPackServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
    dl
    ${CRYPTO_LIBRARY}
    ${SSL_LIBRARY}
    ${ARCHIVE_LIBRARY}
    ${ZLIB_LIBRARY}
)
endif()

if (${MODEMTYPE} STREQUAL "AG591")
target_link_libraries(logPackServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
    dl
    crypto
    ssl
    ${ARCHIVE_LIBRARY}
    ${ZLIB_LIBRARY}
)
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
target_link_libraries(logPackServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    glib-2.0
    gio-2.0
    gobject-2.0
    rt
    pthread
    dl
    crypto
    ssl
    ${ARCHIVE_LIBRARY}
    ${ZLIB_LIBRARY}
)
endif()

# 设置可执行文件名称
set_target_properties(logPackServer PROPERTIES OUTPUT_NAME "logPackServer")

# 安装目标
install(TARGETS logPackServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
