cmake_minimum_required(VERSION 3.0)
project(eraServer)

if (${MODEMTYPE} STREQUAL "A7805")
	file(GLOB ECALL_INC "${PROJECT_SOURCE_DIR}/era_comm/inc" 
				    "${PROJECT_SOURCE_DIR}/era_main/inc"
				    "${PROJECT_SOURCE_DIR}/era_manager/inc"
				    "${PROJECT_SOURCE_DIR}/era_gps/inc"
	                "${PROJECT_SOURCE_DIR}/era_msd_asn/inc"
	              )
	              
	include_directories(
	    ${ECALL_INC}
	    ${PROJECT_SOURCE_DIR}/../../hal/era/include
	    ${PROJECT_SOURCE_DIR}/../../sripc/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/ecall/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/mcu/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/modem/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/param/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc
	)

	file(GLOB ECALL_SRC "${PROJECT_SOURCE_DIR}/era_comm/src/*" 
				  "${PROJECT_SOURCE_DIR}/era_main/src/*"
				  "${PROJECT_SOURCE_DIR}/era_manager/src/*"
				  "${PROJECT_SOURCE_DIR}/era_gps/src/*"
	              "${PROJECT_SOURCE_DIR}/era_msd_asn/src/*"
	              )
	              
	add_executable(eraServer ${ECALL_SRC})

	add_dependencies(eraServer sr-hal)
	add_dependencies(eraServer sr-ipc)
	add_dependencies(eraServer sr-sdk)

	add_subdirectory(era_mock)
# 链接所需的库
	target_link_libraries(eraServer
	    ${ZLOG_LIBRARY}
	    sr-hal
	    sr-ipc
	    sr-sdk
		mtel
	)

# 设置可执行文件名称
	set_target_properties(eraServer PROPERTIES OUTPUT_NAME "eraServer")

# 安装目标
	install(TARGETS eraServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)
endif()
