cmake_minimum_required(VERSION 3.0)

project(sr-sdk)

add_subdirectory(gnss)
add_subdirectory(ecall)
add_subdirectory(param)
add_subdirectory(mcu)
add_subdirectory(doip)
add_subdirectory(pm)
add_subdirectory(modem)
add_subdirectory(docan)
add_subdirectory(uds)
add_subdirectory(net)
add_subdirectory(sdkinit)
add_subdirectory(ota)
add_subdirectory(time)
add_subdirectory(opensslengine)
add_subdirectory(wifi)

if (${MODEMTYPE} STREQUAL "ag35cet")
	add_subdirectory(bcall)	
	include_directories(
        ${PROJECT_SOURCE_DIR}/bcall/include
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/gnss/inc
    ${PROJECT_SOURCE_DIR}/ecall/inc
    ${PROJECT_SOURCE_DIR}/bcall/inc
    ${PROJECT_SOURCE_DIR}/param/inc
    ${PROJECT_SOURCE_DIR}/pm/inc
    ${PROJECT_SOURCE_DIR}/mcu/inc
    ${PROJECT_SOURCE_DIR}/doip/include
    ${PROJECT_SOURCE_DIR}/doip/comm/inc
    ${PROJECT_SOURCE_DIR}/doip/entity/inc
    ${PROJECT_SOURCE_DIR}/doip/tester/inc
    ${PROJECT_SOURCE_DIR}/doip/utils/inc
    ${PROJECT_SOURCE_DIR}/modem/inc
    ${PROJECT_SOURCE_DIR}/param/inc
    ${PROJECT_SOURCE_DIR}/docan/inc
    ${PROJECT_SOURCE_DIR}/uds/inc
    ${PROJECT_SOURCE_DIR}/net/inc
    ${PROJECT_SOURCE_DIR}/sdkinit/inc   
    ${PROJECT_SOURCE_DIR}/../sripc/inc
    ${PROJECT_SOURCE_DIR}/../hal/modem/include
    ${PROJECT_SOURCE_DIR}/ota/inc
    ${PROJECT_SOURCE_DIR}/wifi/inc
    ${PROJECT_SOURCE_DIR}/../service/time/inc
    ${PROJECT_SOURCE_DIR}//time/inc
    ${PROJECT_SOURCE_DIR}/opensslengine/inc
    ${PROJECT_SOURCE_DIR}/../hal/hsm/include
)

# 添加共享库
add_library(sr-sdk SHARED ${SDK_SOURCE_CODE_PATH})

# 链接所需的库
if (${MODEMTYPE} STREQUAL "ag35ce" OR ${MODEMTYPE} STREQUAL "ag35evd")
    target_link_libraries(sr-sdk
        ${ZLOG_LIBRARY}
        sr-ipc
        sr-hal
        glib-2.0
        rt
        pthread
        dl
        crypto
        ssl
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
    )
endif()
if (${MODEMTYPE} STREQUAL "ag35cet" OR ${MODEMTYPE} STREQUAL "ag35cet-openssl3" OR ${MODEMTYPE} STREQUAL "ag35eut")
    target_link_libraries(sr-sdk
        ${ZLOG_LIBRARY}
        sr-ipc
        sr-hal
        ${GLIB2_LIBRARY}
        rt
        pthread
        dl
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
    )
endif()
if (${MODEMTYPE} STREQUAL "A7805")
    target_link_libraries(sr-sdk
        ${ZLOG_LIBRARY}
        sr-ipc
        sr-hal
        ${GLIB2_LIBRARY}
        rt
        pthread
        dl
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
        mtel
    )
endif()
if (${MODEMTYPE} STREQUAL "ag519")
    target_link_libraries(sr-sdk
        ${ZLOG_LIBRARY}
        sr-ipc
        sr-hal
        glib-2.0
        rt
        pthread
        dl
        ${CRYPTO_LIBRARY}
        ${SSL_LIBRARY}
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
    )
endif()
if (${MODEMTYPE} STREQUAL "AG591")
    target_link_libraries(sr-sdk
        ${ZLOG_LIBRARY}
        sr-ipc
        sr-hal
        glib-2.0
        rt
        pthread
        dl
        crypto
        ssl
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
    )
endif()

if (${MODEMTYPE} STREQUAL "ZM8201")
    target_link_libraries(sr-sdk
        ${ZLOG_LIBRARY}
        sr-ipc
        sr-hal
        glib-2.0
        rt
        pthread
        dl
        crypto
        ssl
        ${EVENTCORE_LIBRARY}
        ${EVENTPTHREADS_LIBRARY}
    )
endif()


if (${MODEMTYPE} STREQUAL "ag35cet-openssl3")
    target_compile_options(sr-sdk PRIVATE -Wno-deprecated-declarations)
endif()

# 安装libsr-sdk.so
install(TARGETS sr-sdk DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/)

# 安装sr-sdk头文件
install(FILES ${SDK_INSTALL_HEADER_PATH}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sdk)

add_subdirectory(demo/doipclient)
add_subdirectory(demo/doipserver)
add_subdirectory(demo/udsserver)
add_subdirectory(demo/udsclient)
add_subdirectory(demo/otademo)
add_subdirectory(demo/netclient)
add_subdirectory(demo/pmclient)
add_subdirectory(demo/opensslEngineDemo)
add_subdirectory(demo/gnssdemo)
add_subdirectory(demo/timedemo)
add_subdirectory(demo/doipUpgradeDemo)
add_subdirectory(demo/wifidemo)
add_subdirectory(demo/cantpDemo)
add_subdirectory(demo/kl30ValtageDemo)
add_subdirectory(demo/paramdemo)
