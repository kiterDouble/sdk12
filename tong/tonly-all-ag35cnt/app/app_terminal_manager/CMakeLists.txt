cmake_minimum_required(VERSION 3.0)
project(app)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/build_env.cmake)
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

## add subdirectory
add_subdirectory(terminal_management)
add_subdirectory(terminal_management_main)
