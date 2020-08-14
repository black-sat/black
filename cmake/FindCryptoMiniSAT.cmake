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
## Find the CryptoMiniSAT SAT solver
##

find_library(
  CryptoMiniSAT_LIBRARY
  NAMES libcryptominisat5.so
  PATH_SUFFIXES "/lib/" 
  PATH_SUFFIXES "/lib64/"
)

find_path(
  CryptoMiniSAT_INCLUDE_DIR
  NAMES cryptominisat.h
  PATH_SUFFIXES "/include/cryptominisat5"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CryptoMiniSAT
  FOUND_VAR CryptoMiniSAT_FOUND
  REQUIRED_VARS
    CryptoMiniSAT_LIBRARY
    CryptoMiniSAT_INCLUDE_DIR
  VERSION_VAR CryptoMiniSAT_VERSION
)

if(CryptoMiniSAT_FOUND)

  set(CryptoMiniSAT_LIBRARIES 
    ${CryptoMiniSAT_LIBRARY}
  )
  set(CryptoMiniSAT_INCLUDE_DIRS ${CryptoMiniSAT_INCLUDE_DIR})

  add_library(CryptoMiniSAT_internal UNKNOWN IMPORTED)
  set_target_properties(CryptoMiniSAT_internal PROPERTIES
    IMPORTED_LOCATION "${CryptoMiniSAT_LIBRARY}"
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${CryptoMiniSAT_INCLUDE_DIR}"
  )

  add_library(CryptoMiniSAT INTERFACE)
  target_link_libraries(CryptoMiniSAT INTERFACE CryptoMiniSAT_internal)
  target_include_directories(CryptoMiniSAT SYSTEM INTERFACE ${CryptoMiniSAT_INCLUDE_DIR})

endif()

mark_as_advanced(
  CryptoMiniSAT_INCLUDE_DIR
  CryptoMiniSAT_LIBRARY
)
