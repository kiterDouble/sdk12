cmake_minimum_required(VERSION 3.0)
project(bcallServer)

if (${MODEMTYPE} STREQUAL "ag35cet")

	file(GLOB ECALL_INC "${PROJECT_SOURCE_DIR}/bcall_comm/inc" 
				    "${PROJECT_SOURCE_DIR}/bcall_main/inc"
				    "${PROJECT_SOURCE_DIR}/bcall_manager/inc"
				    "${PROJECT_SOURCE_DIR}/bcall_gps/inc"
	                "${PROJECT_SOURCE_DIR}/bcall_msd_asn/inc"
	              )
	              
	include_directories(
	    ${ECALL_INC}
	    ${PROJECT_SOURCE_DIR}/../../hal/bcall/include
	    ${PROJECT_SOURCE_DIR}/../../hal/ecall/include
	    ${PROJECT_SOURCE_DIR}/../../sripc/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/bcall/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/ecall/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/pm/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/mcu/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/modem/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/param/inc
	    ${PROJECT_SOURCE_DIR}/../../srsdk/sdkinit/inc
	)

	file(GLOB ECALL_SRC "${PROJECT_SOURCE_DIR}/bcall_comm/src/*" 
				  "${PROJECT_SOURCE_DIR}/bcall_main/src/*"
				  "${PROJECT_SOURCE_DIR}/bcall_manager/src/*"
				  "${PROJECT_SOURCE_DIR}/bcall_gps/src/*"
	              "${PROJECT_SOURCE_DIR}/bcall_msd_asn/src/*"
	              )
	              
	add_executable(bcallServer ${ECALL_SRC})

	add_dependencies(bcallServer sr-hal)
	add_dependencies(bcallServer sr-ipc)
	add_dependencies(bcallServer sr-sdk)

	add_subdirectory(bcall_mock)
# 链接所需的库
	target_link_libraries(bcallServer
	    ${ZLOG_LIBRARY}
	    sr-hal
	    sr-ipc
	    sr-sdk
		mtel
	)

# 设置可执行文件名称
	set_target_properties(bcallServer PROPERTIES OUTPUT_NAME "bcallServer")

# 安装目标
	install(TARGETS bcallServer DESTINATION ${CMAKE_INSTALL_PREFIX}/server/)

endif()

