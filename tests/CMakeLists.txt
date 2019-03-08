cmake_minimum_required(VERSION 3.8)

add_library(catch INTERFACE)
target_include_directories(catch INTERFACE ${CMAKE_SOURCE_DIR}/external/catch/single_include/catch2)

set(TESTS test.cpp)

add_executable(tests ${TESTS})
target_link_libraries(tests lib catch)
