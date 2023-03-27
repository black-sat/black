#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2019 - 2020 Nicola Gigante
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


#
# black library
#
set (
  LIB_SRC
  src/support/debug.cpp
  src/logic/logic.cpp
  src/logic/prettyprint.cpp
  src/logic/semantics.cpp
)

# configure config header
configure_file(
  include/black/support/config.hpp.in 
  ${CMAKE_BINARY_DIR}/include/black/support/config.hpp
)

#
# Main black library.
#
add_library (black ${LIB_SRC})

target_link_libraries(black PRIVATE tsl::hopscotch_map)
target_link_libraries(black PUBLIC fmt::fmt cereal)
target_include_directories(black PUBLIC  
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(black PRIVATE src/include)
target_compile_features(black PUBLIC cxx_std_20)
target_enable_warnings(black)
target_code_coverage(black)
add_sanitizers(black)

set_property(TARGET black PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

##
## Installing
##

install(TARGETS black
  EXPORT blackTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(
  DIRECTORY include/black 
  DESTINATION include
  PATTERN "*.in" EXCLUDE
)

install(FILES "${CMAKE_BINARY_DIR}/include/black/support/config.hpp"
  DESTINATION include/black/support
)

install(EXPORT blackTargets
  FILE blackTargets.cmake
  NAMESPACE black::
  DESTINATION lib/cmake/black
)

include(CMakePackageConfigHelpers)

set(INCLUDE_INSTALL_DIR include/)
set(LIBRARY_INSTALL_DIR lib/)

configure_package_config_file(blackConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/blackConfig.cmake
  INSTALL_DESTINATION lib/cmake/black
  PATH_VARS INCLUDE_INSTALL_DIR LIBRARY_INSTALL_DIR)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/blackConfigVersion.cmake
  COMPATIBILITY SameMajorVersion )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/blackConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/blackConfigVersion.cmake
        DESTINATION lib/cmake/black )