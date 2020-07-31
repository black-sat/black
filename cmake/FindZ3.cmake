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
## Find the Z3 SMT solver
##

find_package(PkgConfig)
pkg_check_modules(PC_Z3 QUIET Z3)

find_path(Z3_INCLUDE_DIR
  NAMES z3.h
  PATHS ${PC_Z3_INCLUDE_DIRS}
  PATH_SUFFIXES z3
)
find_library(Z3_LIBRARY
  NAMES z3
  PATHS ${PC_Z3_LIBRARY_DIRS}
)

set(Z3_VERSION ${PC_Z3_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Z3
  FOUND_VAR Z3_FOUND
  REQUIRED_VARS
    Z3_LIBRARY
    Z3_INCLUDE_DIR
  VERSION_VAR Z3_VERSION
)

if(Z3_FOUND)
  set(Z3_LIBRARIES ${Z3_LIBRARY})
  set(Z3_INCLUDE_DIRS ${Z3_INCLUDE_DIR})

  add_library(z3_internal UNKNOWN IMPORTED)
  set_target_properties(z3_internal PROPERTIES
    IMPORTED_LOCATION "${Z3_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_Z3_CFLAGS_OTHER}"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${Z3_INCLUDE_DIR}"
  )

  add_library(Z3 INTERFACE)
  target_link_libraries(Z3 INTERFACE z3_internal)
  target_include_directories(Z3 SYSTEM INTERFACE ${Z3_INCLUDE_DIR})

endif()

mark_as_advanced(
  Z3_INCLUDE_DIR
  Z3_LIBRARY
)
