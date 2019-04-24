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

##
## Glucose SAT solver
##
set(
  GLUCOSE_SRC
  glucose/core/Solver.cc
  glucose/simp/SimpSolver.cc
  glucose/utils/Options.cc
  glucose/utils/System.cc
)

add_library (glucose STATIC ${GLUCOSE_SRC})
target_include_directories(glucose SYSTEM PUBLIC glucose/)

set(
  GLUCOSE_CLANG_WARNINGS
  -Wno-dangling-else
)

target_compile_options(
  glucose PRIVATE
  "$<$<CXX_COMPILER_ID:Clang>:${GLUCOSE_CLANG_WARNINGS}>"
)

##
## MathSAT 5 solver
##

## MathSAT depends on GMP, let's find it first
## TODO: give CMake option to set custom gmp path
find_path(GMP_INCLUDES NAMES gmp.h)
find_path(GMPXX_INCLUDES NAMES gmpxx.h)
find_library(GMP_LIBRARY NAMES gmp)
find_library(GMPXX_LIBRARY NAMES gmpxx)

add_library(gmp SHARED IMPORTED)
set_property(TARGET gmp PROPERTY IMPORTED_LOCATION ${GMP_LIBRARY})

add_library(gmpxx SHARED IMPORTED)
set_property(TARGET gmpxx PROPERTY IMPORTED_LOCATION ${GMPXX_LIBRARY})

## MathSAT imported library
set(MATHSAT_DIR ${CMAKE_SOURCE_DIR}/external/sat/mathsat5)
add_library(mathsat5 STATIC IMPORTED)
set_property(
  TARGET mathsat5
  PROPERTY IMPORTED_LOCATION
    ${MATHSAT_DIR}/lib/libmathsat.a
)

##
## Interface target to link all available solvers
## We set options and directories here instead of the single subtargets because
## of a bug in CMake 3.10 (for Ubuntu 18.04)
##
## TODO: Add CMake flags to choose which solvers to link
##
add_library(solvers INTERFACE)
target_link_libraries(solvers INTERFACE mathsat5 gmpxx gmp)
#target_link_libraries(solvers INTERFACE glucose)
target_include_directories(solvers SYSTEM
  INTERFACE ${MATHSAT_DIR}/include ${GMP_INCLUDES} ${GMPXX_INCLUDES})
