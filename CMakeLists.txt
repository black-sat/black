#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2019 Nicola Gigante
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.10)

project(black LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

# User options
option(ENABLE_TESTS "Enable test suite" ON)
option(STATIC_BUILD "Perform a static build" OFF)
option(ENABLE_GLUCOSE "Enable Glucose SAT solver (still unimplemented)" OFF)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build type not defined. Defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release")
else()
  message(STATUS "Build type: '${CMAKE_BUILD_TYPE}'")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEBUG_ASSERT_DISABLE")

if(STATIC_BUILD)
  set(CMAKE_LINK_FLAGS_DEBUG "${CMAKE_LINK_FLAGS} -static")
  set(CMAKE_LINK_FLAGS_RELEASE "${CMAKE_LINK_FLAGS} -static")
  set(BUILD_SHARED_LIBS OFF)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  message(STATUS "Static build enabled")
endif()

#
# External dependencies
#
option(FMT_PEDANTIC "" ON)
option(FMT_WERROR "" ON)
add_subdirectory(external/fmt)
add_subdirectory(external/debug_assert)
add_subdirectory(external/clipp)

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
  # TODO: warnings ignored for fmt
  -Wno-undef -Wno-missing-noreturn
  -Wno-double-promotion
  # TODO: warnings ignored for clipp
  -Wno-reserved-id-macro
  -Wno-covered-switch-default
)

set(
  GNU_WARNINGS -Wall -Wextra -pedantic -Werror
  -Wno-pragmas -Wno-unknown-pragmas -Wno-unused-but-set-parameter
)

set(MSVC_WARNINGS /wd4068 /wd4702 /W4 /WX)

# Black library and frontend
add_subdirectory(src/lib)
add_subdirectory(src/frontend)

# Tests
if (ENABLE_TESTS)
  enable_testing()

  add_subdirectory(external/catch)
  add_subdirectory(tests)

endif()
