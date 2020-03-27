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

# User options
option(ENABLE_TESTS "Enable test suite" ON)
option(STATIC_BUILD "Perform a static build" OFF)
option(ENABLE_GLUCOSE "Enable Glucose SAT solver (still unimplemented)" OFF)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build configuration not defined. Defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release")
else()
  message(STATUS "Build configuration: '${CMAKE_BUILD_TYPE}'")
endif()

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
set(CMAKE_CXX_STANDARD 17)

option(FMT_PEDANTIC "" ON)
option(FMT_WERROR "" ON)
add_subdirectory(external/fmt)
add_subdirectory(external/debug_assert)
add_subdirectory(external/clipp)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEBUG_ASSERT_DISABLE")

#
# SAT solvers
#
add_subdirectory(external/sat)

#
# Additional modules
#
set(CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/external/sanitizers-cmake/cmake" 
    "${CMAKE_SOURCE_DIR}/cmake"
    ${CMAKE_MODULE_PATH})

find_package(Sanitizers) # Sanitizers
include(EnableWarnings) # Common warnings to enable or disable
include(GNUInstallDirs) # Correct and portable installation paths

# Black library and frontend
add_subdirectory(src/lib)
add_subdirectory(src/frontend)

# Tests
if (ENABLE_TESTS)
  enable_testing()

  add_subdirectory(external/catch)
  add_subdirectory(tests)

endif()
