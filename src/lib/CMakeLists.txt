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


option(ENABLE_Z3 "Enable the Z3 backend, if found" ON)
option(ENABLE_CVC5 "Enable the CVC5 backend, if found" ON)
option(ENABLE_MATHSAT "Enable the MathSAT backend, if found" ON)
option(ENABLE_CMSAT "Enable the CryptoMiniSAT backend, if found" ON)
option(ENABLE_MINISAT "Enable the MiniSAT backend, if found" ON)

set(
  BLACK_DEFAULT_BACKEND "z3"
  CACHE STRING "Default backend called when no -B option is given. Possible values: z3, mathsat, minisat, cmsat"
)

if(
  NOT (BLACK_DEFAULT_BACKEND STREQUAL "z3") AND
  NOT (BLACK_DEFAULT_BACKEND STREQUAL "mathsat") AND
  NOT (BLACK_DEFAULT_BACKEND STREQUAL "minisat") AND
  NOT (BLACK_DEFAULT_BACKEND STREQUAL "cmsat") AND
  NOT (BLACK_DEFAULT_BACKEND STREQUAL "cvc5")
)
  message(
    FATAL_ERROR 
    "Unrecognized value for the BLACK_DEFAULT_BACKEND variable: "
    "'${BLACK_DEFAULT_BACKEND}'\n"
    "Possible values: z3, mathsat, minisat, cmsat"
  )
endif()

if(ENABLE_Z3)
  find_package(Z3)

  if(Z3_FOUND)
    message(STATUS "Z3 solver found! Enabling the Z3 backend...")
  else()
    message(STATUS "Z3 solver not found. Disabling the Z3 backend...")
  endif()
else()
  message(STATUS "Z3 backend disabled.")
endif()

if(ENABLE_CVC5)
  find_package(CVC5)

  if(CVC5_FOUND)
    message(STATUS "CVC5 solver found! Enabling the CVC5 backend...")
  else()
    message(STATUS "CVC5 solver not found. Disabling the CVC5 backend...")
  endif()
else()
  message(STATUS "CVC5 backend disabled.")
endif()

if(ENABLE_MATHSAT)
  find_package(MathSAT)

  if(MathSAT_FOUND)
    message(STATUS "MathSAT solver found! Enabling the MathSAT backend...")
  else()
    message(STATUS "MathSAT solver not found. Disabling the MathSAT backend...")
  endif()
else()
  message(STATUS "MathSAT backend disabled.")
endif()

if(ENABLE_MINISAT)
  find_package(MiniSAT)

  if(MiniSAT_FOUND)
    message(STATUS "MiniSAT solver found! Enabling the MiniSAT backend...")
  else()
    message(STATUS "MiniSAT solver not found. Disabling the MiniSAT backend...")
  endif()
else()
  message(STATUS "MiniSAT backend disabled.")
endif()

if(ENABLE_CMSAT)
  find_package(CryptoMiniSAT)

  if(CryptoMiniSAT_FOUND)
    message(STATUS 
      "CryptoMiniSAT solver found! Enabling the CMSAT backend...")
  else()
    message(STATUS 
      "CryptoMiniSAT solver not found. Disabling the CMSAT backend...")
  endif()
else()
  message(STATUS "CryptoMiniSAT backend disabled.")
endif()

if(NOT Z3_FOUND AND BLACK_DEFAULT_BACKEND STREQUAL "z3")
  message(
    FATAL_ERROR 
    "Z3 set as default backend but not found or disabled.\n"
    "Please install Z3 or set BLACK_DEFAULT_BACKEND differently.")
endif()

if(NOT CVC5_FOUND AND BLACK_DEFAULT_BACKEND STREQUAL "cvc5")
  message(
    FATAL_ERROR 
    "CVC5 set as default backend but not found or disabled.\n"
    "Please install CVC5 or set BLACK_DEFAULT_BACKEND differently.")
endif()

if(NOT MathSAT_FOUND AND BLACK_DEFAULT_BACKEND STREQUAL "mathsat")
  message(
    FATAL_ERROR 
    "MathSAT set as default backend but not found or disabled.\n"
    "Please install MathSAT or set BLACK_DEFAULT_BACKEND differently.")
endif()

if(NOT MiniSAT_FOUND AND BLACK_DEFAULT_BACKEND STREQUAL "minisat")
  message(
    FATAL_ERROR 
    "MiniSAT set as default backend but not found or disabled.\n"
    "Please install MiniSAT or set BLACK_DEFAULT_BACKEND differently.")
endif()

if(NOT CryptoMiniSAT_FOUND AND BLACK_DEFAULT_BACKEND STREQUAL "cmsat")
  message(
    FATAL_ERROR 
    "CryptoMiniSAT set as default backend but not found or disabled.\n"
    "Please install CryptoMiniSAT or set BLACK_DEFAULT_BACKEND differently.")
endif()

if(
  (NOT Z3_FOUND) AND 
  (NOT CVC5_FOUND) AND 
  (NOT MathSAT_FOUND) AND 
  (NOT MiniSAT_FOUND) AND
  (NOT CryptoMiniSAT_FOUND) 
)
  message(
    FATAL_ERROR 
    "No SAT backend found! At least one SAT backend is required for building"
  )
endif()

# configure config header
configure_file(include/black/support/config.hpp.in ${CMAKE_BINARY_DIR}/include/black/support/config.hpp)

#
# black library
#
set (
  LIB_SRC
  src/logic/logic.cpp
  # src/logic/prettyprint.cpp
  # src/logic/formula.cpp
  # src/logic/lex.cpp
  # src/logic/parser.cpp
  # src/logic/past_remover.cpp
  # src/logic/cnf.cpp
  # src/sat/solver.cpp
  # src/sat/dimacs/solver.cpp
  # src/sat/dimacs/parser.cpp
  # src/solver/encoding.cpp
  # src/solver/solver.cpp
  # src/solver/core.cpp
  # src/debug/random_formula.cpp
)

if(Z3_FOUND)
  # set(LIB_SRC ${LIB_SRC} src/sat/backends/z3.cpp)
endif()

if(CVC5_FOUND)
  # set(LIB_SRC ${LIB_SRC} src/sat/backends/cvc5.cpp)
endif()

if(MathSAT_FOUND)
  # set(LIB_SRC ${LIB_SRC} src/sat/backends/mathsat.cpp)
endif()

if(MiniSAT_FOUND)
  # set(LIB_SRC ${LIB_SRC} src/sat/backends/minisat.cpp)
  # set_source_files_properties(
  #   src/sat/backends/minisat.cpp 
  #   PROPERTIES COMPILE_FLAGS -fpermissive
  # )
endif()

if(CryptoMiniSAT_FOUND)
  # set(LIB_SRC ${LIB_SRC} src/sat/backends/cmsat.cpp)
endif()

set (
  LIB_HEADERS
  # include/black/logic/parser.hpp
  # include/black/logic/formula.hpp
  # include/black/logic/lex.hpp
  # include/black/logic/cnf.hpp
  # include/black/logic/alphabet.hpp
  # include/black/logic/past_remover.hpp
  # include/black/internal/formula/match.hpp
  # include/black/internal/formula/base.hpp
  # include/black/internal/formula/impl.hpp
  # include/black/internal/formula/alphabet.hpp
  # include/black/internal/debug/random_formula.hpp
  # include/black/solver/solver.hpp
  # include/black/support/identifier.hpp
  # include/black/support/meta.hpp
  # include/black/support/license.hpp
  # include/black/support/assert.hpp
  # include/black/support/common.hpp
  # include/black/support/range.hpp
  # include/black/support/tribool.hpp
  # include/black/sat/dimacs.hpp
  # include/black/sat/solver.hpp
  # include/black/sat/backends/cmsat.hpp
  # include/black/sat/backends/minisat.hpp
  # include/black/sat/backends/z3.hpp
  # include/black/sat/backends/mathsat.hpp
  # src/include/black/solver/encoding.hpp
)

#
# Main black library.
#
add_library (black ${LIB_SRC} ${LIB_HEADERS})

target_link_libraries(black PRIVATE fmt::fmt tsl::hopscotch_map)
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

if(MathSAT_FOUND)
  target_link_libraries(black PRIVATE MathSAT)
endif()
if(Z3_FOUND)
  target_link_libraries(black PRIVATE Z3)
endif()
if(CVC5_FOUND)
  target_link_libraries(black PRIVATE CVC5)
endif()
if(MiniSAT_FOUND)
  target_link_libraries(black PRIVATE MiniSAT)
endif()
if(CryptoMiniSAT_FOUND)
  target_link_libraries(black PRIVATE CryptoMiniSAT)
endif()

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

configure_file(blackConfig.cmake.in blackConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/blackConfig.cmake"
  DESTINATION lib/cmake/black
)