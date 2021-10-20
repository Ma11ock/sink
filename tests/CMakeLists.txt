
enable_testing()

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)
enable_language(CXX)

project(sink_test)

set(
    SINK_TEST_SRCS
    main.cpp
    )

add_executable(
    sink_test
    ${SINK_TEST_SRCS}
    ${SINK_HEADERS}
    )

target_link_libraries(
    sink_test
    sink
    gtest
    gtest_main
    pthread
    )


# "make check" to compile and run the tests.
add_test(
    NAME    sink_test
    COMMAND sink_test
)

include_directories(${CMAKE_BINARY_TEST_DIR}/src)
target_compile_features(sink_test PRIVATE cxx_std_17)
target_compile_features(sink_test PRIVATE c_std_99)


add_custom_target(check COMMAND sink_test
  DEPENDS sink_test)

