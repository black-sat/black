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

find_package(ZLIB REQUIRED)

##
## Find the MiniSAT SAT solver
##

find_library(
  MiniSAT_LIBRARY
  NAMES libminisat.a minisat
  PATH_SUFFIXES "/lib/"
)

find_path(
  MiniSAT_INCLUDE_DIR
  NAMES simp/SimpSolver.h
  PATH_SUFFIXES "/include/minisat"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MiniSAT
  FOUND_VAR MiniSAT_FOUND
  REQUIRED_VARS
    MiniSAT_LIBRARY
    MiniSAT_INCLUDE_DIR
  VERSION_VAR MiniSAT_VERSION
)

if(MiniSAT_FOUND)

  set(MiniSAT_LIBRARIES 
    ${MiniSAT_LIBRARY} ${ZLIB_LIBRARIES}
  )
  set(MiniSAT_INCLUDE_DIRS ${MiniSAT_INCLUDE_DIR})

  add_library(MiniSAT_internal STATIC IMPORTED)
  set_target_properties(MiniSAT_internal PROPERTIES
    IMPORTED_LOCATION "${MiniSAT_LIBRARY}"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${MiniSAT_INCLUDE_DIR}"
  )

  add_library(MiniSAT INTERFACE)
  target_link_libraries(MiniSAT INTERFACE MiniSAT_internal ZLIB::ZLIB)
  target_include_directories(MiniSAT SYSTEM INTERFACE ${MiniSAT_INCLUDE_DIR})

endif()

mark_as_advanced(
  MiniSAT_INCLUDE_DIR
  MiniSAT_LIBRARY
)
