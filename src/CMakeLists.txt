set(SINK_BINARY_FINAL sink)

enable_language(C)
enable_language(CXX)
set(CMAKE_C_VERSION 99)
set(CMAKE_CXX_VERSION 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_policy(SET CMP0075 NEW)


set(
  SINK_SOURCES
  sink.cpp
  )

set(
  SINK_HEADERS
  sink.hpp
  )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(
  ${SINK_BINARY_FINAL}
  ${SINK_SOURCES}
  ${SINK_HEADERS}
  )


target_compile_features(sink PRIVATE cxx_std_17)
target_compile_features(sink PRIVATE c_std_99)
