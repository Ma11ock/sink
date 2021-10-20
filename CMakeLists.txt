cmake_minimum_required(VERSION 3.20)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE release)
  message("Setting build type to release.")
else()
  set(CMAKE_BUILD_TYPE debug)
  set(SINK_TESTS 1)
  message("Build type is set to debug.")
endif()

project(sink)

add_subdirectory(src)

if(SINK_TESTS AND NOT SINK_NO_TESTS)
  add_subdirectory(tests)
endif()
