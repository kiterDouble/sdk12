
# 设置编译选项
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -D_GNU_SOURCE")
set(CMAKE_INSTALL_RPATH "/oemapp/lib;/oemdata/tts")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -std=c++11 -D_GNU_SOURCE -Wno-deprecated -Wno-psabi")

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

if (${MODEMTYPE} STREQUAL "AG591")
    include_directories(
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/common_inc
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/hal
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/sdk
        ${SDKTARGETSYSROOT}/usr/include/glib-2.0/
        ${SDKTARGETSYSROOT}/usr/include/gio-unix-2.0/
        ${SDKTARGETSYSROOT}/usr/lib/glib-2.0/include/
        ${SDKTARGETSYSROOT}/usr/include/qmi-framework
        ${SDKTARGETSYSROOT}/usr/include/ql-sdk
        ${SDKTARGETSYSROOT}/usr/include/c++/11.4.0/
        ${SDKTARGETSYSROOT}/usr/include/qlsyslog
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../utils/utils-out/include/utils_inc
        ${PROJECT_SOURCE_DIR}/../common_inc
    )
endif()

# 添加头文件目录
if (${MODEMTYPE} STREQUAL "ag35ce")
    include_directories(
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/common_inc
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/hal
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/sdk
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
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include
        ${PROJECT_SOURCE_DIR}/../utils/utils-out/include/utils_inc
    )
endif()

if (${MODEMTYPE} STREQUAL "ag35cet")
    set(CETLIB $ENV{CETLIB})
    message("CETLIB: ${CETLIB}")
    include_directories(
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/common_inc
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/hal
        ${PROJECT_SOURCE_DIR}/../server/server-out/include/sdk
        ${PROJECT_SOURCE_DIR}/../common_inc
        ${CETLIB}/include/glib-2.0/
        ${CETLIB}/include/gio-unix-2.0/
        ${CETLIB}/lib/glib-2.0/include/
        ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/include/
        ${PROJECT_SOURCE_DIR}/../utils/utils-out/include/utils_inc
#        /opt/ql_crosstools/ql-ag35-1806e-gcc-8.4.0-glibc-v1-toolchain/arm-openwrt-linux-gnueabi/include/c++/8.4.0/
	    ${SDKTARGETSYSROOT}/usr/include/ql-sdk
    )
endif()

link_directories(
    ${PROJECT_SOURCE_DIR}/../server/server-out/lib
)


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
find_library(ZLOG_LIBRARY zlog  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(FLATCC_LIBRARY flatccrt  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(UTILS_LIBRARY sr_utils  ${PROJECT_SOURCE_DIR}/../utils/utils-out/lib/)
find_library(EVENTCORE_LIBRARY event_core  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(EVENTPTHREADS_LIBRARY event_pthreads  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(BOOST_SYSTEM_LIBRARIES boost_system  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(VSOMEIP_LIBRARIES vsomeip3  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(VSOMEIP_CFG_LIBRARIES vsomeip3-cfg  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(VSOMEIP_SD_LIBRARIES vsomeip3-sd  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(MQTT_LIBRARY paho-mqtt3cs  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(MQTT_ASYNC_LIBRARY paho-mqtt3as  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(CIVETWEB libcivetweb.a  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(ICONV iconv  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
find_library(ARCHIVE_LIBRARY archive  ${PROJECT_SOURCE_DIR}/../thrid-lib/thrid-lib-out/lib/)
if (${MODEMTYPE} STREQUAL "ag35cet")
    find_library(GLIB2_LIBRARY glib-2.0  ${CETLIB}/lib/)
    find_library(GIO_LIBRARY gio-2.0  ${CETLIB}/lib/)
    find_library(GOBJECT_LIBRARY gobject-2.0  ${CETLIB}/lib/)
    find_library(FFI_LIBRARY ffi  ${CETLIB}/lib/)
    find_library(GMODULE2_LIBRARY gmodule-2.0  ${CETLIB}/lib/)
    find_library(CRYPTO_LIBRARY crypto  ${CETLIB}/lib/)
    find_library(SSL_LIBRARY ssl  ${CETLIB}/lib/)
    find_library(CURL_LIBRARY curl  ${CETLIB}/lib/)
    find_library(PROTOBUF_LIBRARY protobuf  ${CETLIB}/lib/)
endif()

if (${MODEMTYPE} STREQUAL "AG591")
    find_library(GLIB2_LIBRARY glib-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(GIO_LIBRARY gio-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(GOBJECT_LIBRARY gobject-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(FFI_LIBRARY ffi  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(GMODULE2_LIBRARY gmodule-2.0  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(CRYPTO_LIBRARY crypto  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(SSL_LIBRARY ssl  ${SDKTARGETSYSROOT}/usr/lib/)
    find_library(CURL_LIBRARY curl ${SDKTARGETSYSROOT}/usr/lib/)
    #find_library(PROTOBUF_LIBRARY protobuf  ${CETLIB}/lib/)
endif()
