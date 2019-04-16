cmake_minimum_required(VERSION 3.8)

project(black)

#
# External dependencies
#
add_subdirectory(external/debug_assert)
add_subdirectory(external/fmt)
add_subdirectory(external/remotery)

#
# SAT solvers
#
add_subdirectory(external/sat)

#
# Warnings
#
set(
  CLANG_WARNINGS -Weverything -pedantic -Werror
  -Wno-c++98-compat -Wno-c++98-compat-pedantic
  -Wno-c++98-c++11-c++14-compat
  -Wno-padded -Wno-weak-vtables -Wno-unknown-pragmas
  -Wno-exit-time-destructors -Wno-switch-enum
  -Wno-undefined-var-template
  -Wno-undefined-func-template
  -Wno-deprecated
  -Wno-old-style-cast -Wno-documentation
  -Wno-documentation-unknown-command
  -Wno-sign-conversion
  -Wno-global-constructors -Wno-extra-semi
  -Wno-unknown-warning-option
  # TODO: Fix fmt to re-enable these warnings
  -Wno-undef -Wno-missing-noreturn
  -Wno-double-promotion
)

set(
  GNU_WARNINGS -Wall -Wextra -pedantic -Werror
  -Wno-pragmas -Wno-unknown-pragmas -Wno-unused-but-set-parameter
)

set(MSVC_WARNINGS /wd4068 /wd4702 /W4 /WX)

# Black library and executble
add_subdirectory(src/lib)
add_subdirectory(src/executable)

# Tests
add_subdirectory(tests/units)
