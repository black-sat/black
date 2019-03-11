cmake_minimum_required(VERSION 3.8)

#
# Main black executable
#
add_executable(executable main.cpp)

target_link_libraries(executable black)
target_compile_features(executable PRIVATE cxx_std_17)
target_link_libraries(executable debug_assert fmt)
target_compile_options(
  executable PRIVATE
  "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
)


set_property(TARGET executable PROPERTY OUTPUT_NAME black)
set_property(TARGET executable PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
