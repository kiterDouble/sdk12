cmake_minimum_required(VERSION 3.0)
project(app)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_env.cmake)

include_directories(
    ${PROJECT_SOURCE_DIR}/comFunc/inc
    ${PROJECT_SOURCE_DIR}/pki_sdk_lib/inc
)

## add subdirectory
#add_subdirectory(sqlite3x)
add_subdirectory(comFunc)
add_subdirectory(audio)
add_subdirectory(otamodem1)
add_subdirectory(otamodem2)
add_subdirectory(ethernet_upgrade)
add_subdirectory(can_file_save)
add_subdirectory(pki_cert)
add_subdirectory(pki_sdk_lib)
add_subdirectory(ota_configuration)
add_subdirectory(remote_monitor)
add_subdirectory(mqtt_proto)
add_subdirectory(canmsg)
add_subdirectory(canmsgDemo)
add_subdirectory(cantp)
add_subdirectory(carota-server)
add_subdirectory(doiptp)
#add_subdirectory(demo/imokdemo)

