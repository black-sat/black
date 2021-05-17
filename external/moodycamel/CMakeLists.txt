cmake_minimum_required(VERSION 3.10)
project(concurrentqueue VERSION 1.0.0)

add_library(concurrentqueue INTERFACE)

target_include_directories(
    concurrentqueue INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)