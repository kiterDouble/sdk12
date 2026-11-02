
# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -D_GNU_SOURCE")

# 让崩溃时的 backtrace 输出可通过 addr2line 定位源码位置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -fno-omit-frame-pointer -funwind-tables -fasynchronous-unwind-tables -rdynamic")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -g -O2 -fno-omit-frame-pointer -funwind-tables -fasynchronous-unwind-tables -rdynamic")

# 设置RUN PATH
set(CMAKE_INSTALL_RPATH "/oemapp/lib")

set(SDKTARGETSYSROOT $ENV{SDKTARGETSYSROOT})
message("SDKTARGETSYSROOT: ${SDKTARGETSYSROOT}")

set(MODEMTYPE $ENV{MODEMTYPE})
message("MODEMTYPE: ${MODEMTYPE}")

execute_process(
    COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_VARIABLE GCC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("GCC Version: ${GCC_VERSION}")

# 添加头文件目录
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
    include_directories(
        ${PROJECT_SOURCE_DIR}/common_inc
        ${SDKTARGETSYSROOT}/usr/include/
        ${SDKTARGETSYSROOT}/usr/include/c++/${GCC_VERSION}/
        ${SDKTARGETSYSROOT}/usr/include/c++/${GCC_VERSION}/arm-oe-linux-gnueabi/
        ${SDKTARGETSYSROOT}/usr/include/glib-2.0/
        ${SDKTARGETSYSROOT}/usr/include/gio-unix-2.0/
        ${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include/
        ${SDKTARGETSYSROOT}/usr/include/qmi
        ${SDKTARGETSYSROOT}/usr/include/data/
        ${SDKTARGETSYSROOT}/usr/include/dsutils/
        ${SDKTARGETSYSROOT}/usr/include/qmi-framework/
        ${SDKTARGETSYSROOT}/usr/include/quectel-openlinux-sdk/
        ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/utils-out/include/utils_inc
    )
endif()

if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35eut")
    include_directories(
        ${PROJECT_SOURCE_DIR}/common_inc
        ${SDKTARGETSYSROOT}/usr/include/ql-sdk
        ${SDKTARGETSYSROOT}/usr/include/glib-2.0/
        ${SDKTARGETSYSROOT}/usr/include/gio-unix-2.0/
        ${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include/
        ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/utils-out/include/utils_inc
    )
endif()

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
    include_directories(
        ${PROJECT_SOURCE_DIR}/common_inc
        ${SDKTARGETSYSROOT}/usr/include/ql-sdk
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include/glib-2.0/
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include/gio-unix-2.0/
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/glib-2.0/include/
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../utils/utils-out/include/utils_inc
    )
endif()

if (${MODEMTYPE} STREQUAL "A7805")
    set(LIB_PATH $ENV{LIB_PATH})
    set(CROSSTOOL_PATH $ENV{CROSSTOOL_PATH})
    message("A7805 SDKTARGETSYSROOT: ${LIB_PATH}")
    message("A7805 CROSSTOOL_PATH: ${CROSSTOOL_PATH}")
    include_directories(
        ${PROJECT_SOURCE_DIR}/common_inc
        ${CROSSTOOL_PATH}/../../A7805lib/include/glib-2.0/
        ${CROSSTOOL_PATH}/../../A7805lib/include/gio-unix-2.0/
        ${CROSSTOOL_PATH}/../../A7805lib/include/
        ${CROSSTOOL_PATH}/../../A7805lib/lib/glib-2.0/include/
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../utils/utils-out/include/utils_inc
        ${LIB_PATH}/usr/include/
    )
endif()

if (${MODEMTYPE} STREQUAL "ag519")
    include_directories(
        ${PROJECT_SOURCE_DIR}/common_inc
        ${SDKTARGETSYSROOT}/usr/include/
        ${SDKTARGETSYSROOT}/usr/include/c++/8.2.0/
        ${SDKTARGETSYSROOT}/usr/include/c++/8.2.0/arm-poky-linux-gnueabi/
        ${SDKTARGETSYSROOT}/usr/include/glib-2.0/
        ${SDKTARGETSYSROOT}/usr/include/gio-unix-2.0/
        ${SDKTARGETSYSROOT}/usr/include/ql-location/
        ${SDKTARGETSYSROOT}/usr/include/ql-sdk/
        ${SDKTARGETSYSROOT}/usr/include/ql_lib_ipc/
        ${SDKTARGETSYSROOT}/usr/include/ql_lib_mtkutils/
        ${SDKTARGETSYSROOT}/usr/include/ql_lib_utils/
        ${SDKTARGETSYSROOT}/usr/include/qlsyslog/
        ${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include/
        ${SDKTARGETSYSROOT}/usr/include/
        ${SDKTARGETSYSROOT}/../../ag519lib/include/
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../utils/utils-out/include/utils_inc
    )
endif()
if (${MODEMTYPE} STREQUAL "AG591")
    include_directories(
        ${PROJECT_SOURCE_DIR}/common_inc
        ${SDKTARGETSYSROOT}/usr/include/glib-2.0/
        ${SDKTARGETSYSROOT}/usr/include/gio-unix-2.0/
        ${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include/
        ${SDKTARGETSYSROOT}/usr/include/qmi-framework
        ${SDKTARGETSYSROOT}/usr/include/ql-sdk
        ${SDKTARGETSYSROOT}/usr/include/c++/11.4.0/
        ${SDKTARGETSYSROOT}/usr/include/qlsyslog
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../utils/utils-out/include/utils_inc
    )
endif()

get_property(INCLUDE_DIRS DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
set(MISSING_DIRS "")

foreach(INCLUDE_DIR ${INCLUDE_DIRS})
    if(NOT EXISTS ${INCLUDE_DIR})
        message(FATAL_ERROR "Include directory does not exist: ${INCLUDE_DIR}")
        list(APPEND MISSING_DIRS ${INCLUDE_DIR})
    endif()
endforeach()

string(REPLACE ";" "\n" INCLUDE_DIRS_STR "${INCLUDE_DIRS}")
message("include_directories:\n${INCLUDE_DIRS_STR}")

if(MISSING_DIRS)
    string(REPLACE ";" "\n" MISSING_DIRS_STR "${MISSING_DIRS}")
    message("Missing include directories:\n${MISSING_DIRS_STR}")
endif()

## 设置动态库选型
find_library(ZLOG_LIBRARY zlog  ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/thrid-lib-out/lib/)
# find_library(FLATCC_LIBRARY flatccrt  ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/thrid-lib-out/lib/)
find_library(UTILS_LIBRARY sr_utils  ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/utils-out/lib/)
find_library(EVENTCORE_LIBRARY event_core  ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/thrid-lib-out/lib/)
find_library(EVENTPTHREADS_LIBRARY event_pthreads  ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/thrid-lib-out/lib/)
find_library(ARCHIVE_LIBRARY archive ${PROJECT_SOURCE_DIR}/../C3BaseLineOut/thrid-lib-out/lib/)

if (${MODEMTYPE} STREQUAL "ZM8201")
    include_directories(
        ${PROJECT_SOURCE_DIR}/common_inc
        ${SDKTARGETSYSROOT}/usr/include/glib-2.0/
        ${SDKTARGETSYSROOT}/usr/include/gio-unix-2.0/
        ${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include/
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../utils/utils-out/include/utils_inc
    )

    link_directories(
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/
        ${PROJECT_SOURCE_DIR}/../utils/utils-out/lib/
        ${SDKTARGETSYSROOT}/usr/lib/
    )

    set(ZLOG_LIBRARY zlog)
    set(UTILS_LIBRARY sr_utils)
    set(EVENTCORE_LIBRARY event_core)
    set(EVENTPTHREADS_LIBRARY event_pthreads)
    set(GLIB2_LIBRARY glib-2.0)
    set(GIO_LIBRARY gio-2.0)
    set(GOBJECT_LIBRARY gobject-2.0)
    set(FFI_LIBRARY ffi)
    set(GMODULE2_LIBRARY gmodule-2.0)

endif()

if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35eut")
    find_library(GLIB2_LIBRARY glib-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(GIO_LIBRARY gio-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(GOBJECT_LIBRARY gobject-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(FFI_LIBRARY ffi  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(GMODULE2_LIBRARY gmodule-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
    # find_library(CRYPTO_LIBRARY crypto  ${SDKTARGETSYSROOT}/lib/)
    # find_library(SSL_LIBRARY ssl ${SDKTARGETSYSROOT}/lib/)
    find_library(CRYPTO_LIBRARY crypto  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(SSL_LIBRARY ssl ${SDKTARGETSYSROOT}/usr/lib/)
endif()

if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
    find_library(GLIB2_LIBRARY glib-2.0  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
    find_library(GIO_LIBRARY gio-2.0  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
    find_library(GOBJECT_LIBRARY gobject-2.0  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
    find_library(FFI_LIBRARY ffi  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
    find_library(GMODULE2_LIBRARY gmodule-2.0  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
    find_library(CRYPTO_LIBRARY crypto  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
    find_library(SSL_LIBRARY ssl ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
    find_library(CURL_LIBRARY curl ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
    find_library(Z_LIBRARY z ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
endif()

if (${MODEMTYPE} STREQUAL "A7805")
    find_library(GLIB2_LIBRARY glib-2.0  ${CROSSTOOL_PATH}/../../A7805lib/lib)
    find_library(GIO_LIBRARY gio-2.0  ${CROSSTOOL_PATH}/../../A7805lib/lib)
    find_library(GOBJECT_LIBRARY gobject-2.0  ${CROSSTOOL_PATH}/../../A7805lib/lib)
    find_library(FFI_LIBRARY ffi  ${CROSSTOOL_PATH}/../../A7805lib/lib)
    find_library(GMODULE2_LIBRARY gmodule-2.0  ${CROSSTOOL_PATH}/../../A7805lib/lib)
    find_library(CRYPTO_LIBRARY crypto  ${CROSSTOOL_PATH}/../../A7805lib/lib)
    find_library(SSL_LIBRARY ssl ${CROSSTOOL_PATH}/../../A7805lib/lib)
    find_library(SQLITE3_LIBRARY sqlite3 ${CROSSTOOL_PATH}/../../A7805lib/lib)
    find_library(PCRE_LIBRARY pcre ${CROSSTOOL_PATH}/../../A7805lib/lib)
    link_directories(${LIB_PATH}/usr/lib)
endif()

if (${MODEMTYPE} STREQUAL "ag519")
    find_library(CRYPTO_LIBRARY crypto  ${SDKTARGETSYSROOT}/../../ag519lib/lib)
    find_library(SSL_LIBRARY ssl ${SDKTARGETSYSROOT}/../../ag519lib/lib)
endif()
