cmake_minimum_required(VERSION 3.8)

add_library(catch INTERFACE)
target_include_directories(catch INTERFACE ${CMAKE_SOURCE_DIR}/external/catch/single_include/catch2)

set(
  TESTS
  main.cpp
  common.cpp
  formula.cpp
  #solver.cpp
)

add_executable(tests ${TESTS})
target_link_libraries(tests black catch)

target_compile_options(
  tests PUBLIC
  "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
)
