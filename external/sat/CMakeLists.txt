cmake_minimum_required(VERSION 3.8)

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
target_include_directories(glucose PUBLIC glucose/)

set(
  GLUCOSE_CLANG_WARNINGS
  -Wno-logical-op-parentheses
  -Wno-dangling-else
)

target_compile_options(
  glucose PRIVATE
  # "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  # "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${GLUCOSE_CLANG_WARNINGS}>"
)

##
## MathSAT 5 solver
##
add_library(mathsat5 STATIC IMPORTED)
set_property(
  TARGET mathsat5
  PROPERTY IMPORTED_LOCATION
    ${CMAKE_SOURCE_DIR}/external/sat/mathsat5/lib/libmathsat.a
)
target_link_libraries(mathsat5 INTERFACE gmpxx gmp)
target_include_directories(mathsat5 INTERFACE mathsat5/include/)

##
## Interface target to link all available solvers
##
## TODO: Add CMake flags to choose which solvers to link
##
add_library(solvers INTERFACE)
target_link_libraries(solvers INTERFACE glucose mathsat5)
