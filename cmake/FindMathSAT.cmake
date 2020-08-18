#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2020 Nicola Gigante
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

find_package(GMP)
find_package(ZLIB)

##
## Find the MathSAT 5 SMT solver
##

# Get the current architecture to compose the mathsat directory name
# used when keeping mathsat in-tree.
execute_process(
  COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE CURRENT_ARCH
)

set(MATHSAT_ARCH_NAME "mathsat5-${CMAKE_SYSTEM_NAME}-${CURRENT_ARCH}")

find_library(
  MathSAT_LIBRARY
  NAMES libmathsat.a mathsat
  PATHS "${CMAKE_SOURCE_DIR}/external/mathsat5/"
        "${CMAKE_SOURCE_DIR}/external/${MATHSAT_ARCH_NAME}/"
  PATH_SUFFIXES "/lib/"
)

find_path(
  MathSAT_INCLUDE_DIR
  NAMES mathsat.h
  PATHS "${CMAKE_SOURCE_DIR}/external/mathsat5"
        "${CMAKE_SOURCE_DIR}/external/${MATHSAT_ARCH_NAME}/"
  PATH_SUFFIXES "/include/"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MathSAT
  FOUND_VAR MathSAT_FOUND
  REQUIRED_VARS 
    MathSAT_LIBRARY
    MathSAT_INCLUDE_DIR
    GMP_LIBRARIES
    ZLIB_LIBRARIES
  VERSION_VAR MathSAT_VERSION
)

if(MathSAT_FOUND)

  set(MathSAT_LIBRARIES 
    ${MathSAT_LIBRARY} ${GMP_LIBRARIES} ${ZLIB_LIBRARIES}
  )
  set(MathSAT_INCLUDE_DIRS ${MathSAT_INCLUDE_DIR})

  add_library(MathSAT_internal STATIC IMPORTED)
  set_target_properties(MathSAT_internal PROPERTIES
    IMPORTED_LOCATION "${MathSAT_LIBRARY}"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${MathSAT_INCLUDE_DIR}"
  )

  add_library(MathSAT INTERFACE)
  target_link_libraries(MathSAT INTERFACE MathSAT_internal GMP GMPXX ZLIB::ZLIB)
  target_include_directories(MathSAT SYSTEM INTERFACE ${MathSAT_INCLUDE_DIR})

endif()

mark_as_advanced(
  MathSAT_INCLUDE_DIR
  MathSAT_LIBRARY
)
