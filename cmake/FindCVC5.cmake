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
## Find the CVC5 SMT solver
##

find_package(PkgConfig)
pkg_check_modules(PC_CVC5 QUIET CVC5)

find_path(CVC5_INCLUDE_DIR
  NAMES cvc5.h
  PATHS ${PC_CVC5_INCLUDE_DIRS}
  PATH_SUFFIXES cvc5
)
find_library(CVC5_LIBRARY
  NAMES cvc5
  PATHS ${PC_CVC5_LIBRARY_DIRS}
)

set(CVC5_VERSION ${PC_CVC5_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CVC5
  FOUND_VAR CVC5_FOUND
  REQUIRED_VARS
    CVC5_LIBRARY
    CVC5_INCLUDE_DIR
  VERSION_VAR CVC5_VERSION
)

if(CVC5_FOUND)
  set(CVC5_LIBRARIES ${CVC5_LIBRARY})
  set(CVC5_INCLUDE_DIRS ${CVC5_INCLUDE_DIR})

  add_library(CVC5_internal UNKNOWN IMPORTED)
  set_target_properties(CVC5_internal PROPERTIES
    IMPORTED_LOCATION "${CVC5_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_CVC5_CFLAGS_OTHER}"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CVC5_INCLUDE_DIR}"
  )

  add_library(CVC5 INTERFACE)
  target_link_libraries(CVC5 INTERFACE CVC5_internal)
  target_include_directories(CVC5 SYSTEM INTERFACE ${CVC5_INCLUDE_DIR})

endif()

mark_as_advanced(
  CVC5_INCLUDE_DIR
  CVC5_LIBRARY
)
