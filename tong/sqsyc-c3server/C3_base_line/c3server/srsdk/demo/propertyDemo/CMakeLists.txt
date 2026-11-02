cmake_minimum_required(VERSION 3.0)
project(propertyDemo)

include_directories(
    ${PROJECT_SOURCE_DIR}/../../property/inc
)

add_executable(tboxPropertyGet tbox_property_get.c)
target_compile_options(tboxPropertyGet PRIVATE -Wno-error)
target_link_libraries(tboxPropertyGet
    sr-sdk
)
set_target_properties(tboxPropertyGet PROPERTIES OUTPUT_NAME "tboxPropertyGet")
install(TARGETS tboxPropertyGet DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)

add_executable(tboxPropertySet tbox_property_set.c)
target_compile_options(tboxPropertySet PRIVATE -Wno-error)
target_link_libraries(tboxPropertySet
    sr-sdk
)
set_target_properties(tboxPropertySet PROPERTIES OUTPUT_NAME "tboxPropertySet")
install(TARGETS tboxPropertySet DESTINATION ${CMAKE_INSTALL_PREFIX}/demo/)
