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

cmake_minimum_required(VERSION 3.25)

project(
  black 
  VERSION 25.03.0
  LANGUAGES C CXX
  DESCRIPTION "BLACK (Bounded Lᴛʟ sAtisfiability ChecKer)"
)

# User options
option(ENABLE_TESTS "Enable test suite" ON)
option(ENABLE_FORMULAS_TESTS "Enable the formulas test suite" ON)
option(ENABLE_CLANG_TIDY "Enable Clang Tidy invocation" OFF)

set(CLANG_TIDY_PATH "clang-tidy" CACHE FILEPATH "PATH to clang-tidy executable")
if(ENABLE_CLANG_TIDY)
  message(STATUS "Clang Tidy enabled")
  set(
    CMAKE_CXX_CLANG_TIDY 
    ${CLANG_TIDY_PATH} -header-filter=. -warnings-as-errors=* 
    -checks=clang-analyzer-*,portability-*,bugprone-*,-bugprone-forwarding-reference-overload)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  message(STATUS "Build configuration not defined. Defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release")
else()
  message(STATUS "Build configuration: '${CMAKE_BUILD_TYPE}'")
endif()

set(BUILD_SHARED_LIBS ON)

#
# Additional modules
#
set(CMAKE_MODULE_PATH
    "${CMAKE_SOURCE_DIR}/external/sanitizers-cmake/cmake" 
    "${CMAKE_SOURCE_DIR}/cmake"
    ${CMAKE_MODULE_PATH})

include(GitSubmodules) # To automatically fetch dependencies
include(EnableWarnings) # Common warnings to enable or disable
include(CodeCoverage) # Code to enable code coverage instrumentation
include(GNUInstallDirs) # Correct and portable installation paths
include(CheckIPOSupported) # Check if we can do LTO

update_git_submodules(FATAL) # Fetch git submodules

find_package(Sanitizers) # Sanitizers

#
# Fix the RPATH for the installation 
#
if(APPLE)
  set(CMAKE_MACOSX_RPATH 1)
  list(
    APPEND CMAKE_INSTALL_RPATH 
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
  )
endif()

#
# External dependencies
#
find_package(tsl-hopscotch-map REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(external/clipp EXCLUDE_FROM_ALL)

# mark the depenencies to ignore warnings from their interface headers
target_mark_as_system(clipp)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DBLACK_ASSERT_DISABLE")
check_ipo_supported(RESULT IPO_SUPPORTED LANGUAGES C CXX)
if(IPO_SUPPORTED)
  message(STATUS "Link-Time Optimization enabled")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE YES)
endif()

# Black library and frontend
add_subdirectory(src/lib)
add_subdirectory(src/frontend)

# Tests

if (ENABLE_TESTS)
  
  enable_testing()
  add_subdirectory(tests)

endif()


##
## Packaging
##
set(CPACK_PACKAGE_NAME "black-sat")
set(CPACK_PACKAGE_VENDOR "The BLACK team")
set(CPACK_PACKAGE_CONTACT "Nicola Gigante <nicola.gigante@gmail.com>")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

set(CPACK_DEBIAN_PACKAGE_SECTION "science")
set(CPACK_DEBIAN_PACKAGE_DEPENDS 
  "libc6,libgcc1,libstdc++6,zlib1g,libz3-4,libfmt-dev"
)

include(CPack)