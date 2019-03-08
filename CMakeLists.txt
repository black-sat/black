cmake_minimum_required(VERSION 3.8)

project(black)

#
# External dependencies
#
add_subdirectory(external/debug_assert)
add_subdirectory(external/fmt)

# Black library
add_subdirectory(src/lib)

# Tests
add_subdirectory(tests)

#
# Main black executable
#
set(black_sources
  ${CMAKE_SOURCE_DIR}/src/black/main.cpp)

add_executable(black ${black_sources})

target_link_libraries(black lib)
target_compile_features(black PRIVATE cxx_std_17)
target_include_directories(black PRIVATE ${CMAKE_SOURCE_DIR}/src/black/include/)

# External dependencies
target_link_libraries(black debug_assert fmt)
