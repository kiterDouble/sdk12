cmake_minimum_required(VERSION 3.0)
project(ecallServer)


file(GLOB ECALL_INC "${PROJECT_SOURCE_DIR}/ecall_comm/inc" 
			    "${PROJECT_SOURCE_DIR}/ecall_main/inc"
			    "${PROJECT_SOURCE_DIR}/ecall_manager/inc"
			    "${PROJECT_SOURCE_DIR}/ecall_gps/inc"
                "${PROJECT_SOURCE_DIR}/ecall_msd_asn/inc"
              )
              
include_directories(
    ${ECALL_INC}
    ${PROJECT_SOURCE_DIR}/../../hal/ecall/include
    ${PROJECT_SOURCE_DIR}/../../sripc/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/ecall/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/mcu/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/modem/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/param/inc
    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc
)

file(GLOB ECALL_SRC "${PROJECT_SOURCE_DIR}/ecall_comm/src/*" 
			  "${PROJECT_SOURCE_DIR}/ecall_main/src/*"
			  "${PROJECT_SOURCE_DIR}/ecall_manager/src/*"
			  "${PROJECT_SOURCE_DIR}/ecall_gps/src/*"
              "${PROJECT_SOURCE_DIR}/ecall_msd_asn/src/*"
              )
              
add_executable(ecallServer ${ECALL_SRC})

add_dependencies(ecallServer sr-hal)
add_dependencies(ecallServer sr-ipc)
add_dependencies(ecallServer sr-sdk)

add_subdirectory(ecall_mock)
# 链接所需的库
if (${MODEMTYPE} STREQUAL "A7805")
target_link_libraries(ecallServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
    mtel
)
else()
target_link_libraries(ecallServer
    ${ZLOG_LIBRARY}
    sr-hal
    sr-ipc
    sr-sdk
)
endif()

# 设置可执行文件名称
set_target_properties(ecallServer PROPERTIES OUTPUT_NAME "ecallServer")

# 安装目标
install(TARGETS ecallServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)


