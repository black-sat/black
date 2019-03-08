cmake_minimum_required(VERSION 3.8)

add_library(lib INTERFACE)
target_link_libraries(lib INTERFACE debug_assert fmt)
target_include_directories(lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(lib INTERFACE cxx_std_17)
