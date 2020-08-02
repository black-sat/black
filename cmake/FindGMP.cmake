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

##
## Find the GMP SMT solver
##

find_package(PkgConfig)
pkg_check_modules(PC_GMP QUIET GMP)

find_path(GMP_INCLUDE_DIR
  NAMES gmp.h
  PATHS ${PC_GMP_INCLUDE_DIRS}
)

find_path(GMPXX_INCLUDE_DIR
  NAMES gmpxx.h
  PATHS ${PC_GMP_INCLUDE_DIRS}
)

find_library(GMP_LIBRARY
  NAMES gmp
  PATHS ${PC_GMP_LIBRARY_DIRS}
)

find_library(GMPXX_LIBRARY
  NAMES gmpxx
  PATHS ${PC_GMP_LIBRARY_DIRS}
)

set(GMP_VERSION ${PC_GMP_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP
  FOUND_VAR GMP_FOUND
  REQUIRED_VARS
    GMP_LIBRARY
    GMP_INCLUDE_DIR
    GMPXX_LIBRARY
    GMPXX_INCLUDE_DIR
  VERSION_VAR GMP_VERSION
)

if(GMP_FOUND)
  set(GMP_LIBRARIES ${GMP_LIBRARY} ${GMPXX_LIBRARY})
  set(GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR} {GMPXX_INCLUDE_DIR})

  add_library(GMP_internal UNKNOWN IMPORTED)
  set_target_properties(GMP_internal PROPERTIES
    IMPORTED_LOCATION "${GMP_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_GMP_CFLAGS_OTHER}"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}"
  )
  add_library(GMPXX_internal UNKNOWN IMPORTED)
  set_target_properties(GMPXX_internal PROPERTIES
    IMPORTED_LOCATION "${GMPXX_LIBRARY}"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${GMPXX_INCLUDE_DIR}"
  )

  add_library(GMP INTERFACE)
  target_link_libraries(GMP INTERFACE GMP_internal)
  target_include_directories(GMP SYSTEM INTERFACE ${GMP_INCLUDE_DIR})
  
  add_library(GMPXX INTERFACE)
  target_link_libraries(GMPXX INTERFACE GMPXX_internal)
  target_include_directories(GMPXX SYSTEM INTERFACE ${GMPXX_INCLUDE_DIR})  

endif()

mark_as_advanced(
  GMP_INCLUDE_DIR
  GMP_LIBRARY
)
