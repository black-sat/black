cmake_minimum_required(VERSION 3.8)

add_library(lib INTERFACE)
target_link_libraries(lib INTERFACE debug_assert fmt)
target_compile_options(lib INTERFACE -Weverything)
target_include_directories(lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(lib INTERFACE cxx_std_17)

# Warnings
set(
  CLANG_WARNINGS -Weverything -pedantic -Werror
  -Wno-c++98-compat -Wno-c++98-compat-pedantic
  -Wno-padded -Wno-weak-vtables -Wno-unknown-pragmas
  -Wno-exit-time-destructors -Wno-switch-enum
  -Wno-undefined-var-template
  -Wno-undefined-func-template
  -Wno-reserved-id-macro -Wno-deprecated
  -Wno-old-style-cast -Wno-documentation
  -Wno-documentation-unknown-command
  -Wno-sign-conversion
  -Wno-global-constructors -Wno-extra-semi
  -Wno-unknown-warning-option
  # TODO: Fix fmt to re-enable these warnings
  -Wno-undef -Wno-missing-noreturn
)

set(
  GNU_WARNINGS -Wall -Wextra -pedantic -Werror
  -Wno-pragmas -Wno-unknown-pragmas
)

set(MSVC_WARNINGS /wd4068 /wd4702 /W4 /WX)

target_compile_options(
  lib INTERFACE
  "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
)
