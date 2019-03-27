cmake_minimum_required(VERSION 3.8)

set(
  GLUCOSE_SRC
  glucose/core/Solver.cc
  glucose/simp/SimpSolver.cc
  glucose/utils/Options.cc
  glucose/utils/System.cc
)

add_library (solvers STATIC ${GLUCOSE_SRC})
target_include_directories(solvers PUBLIC glucose/)
target_include_directories(solvers PUBLIC mathsat5/include/)

set(
  GLUCOSE_CLANG_WARNINGS
  -Wno-logical-op-parentheses
  -Wno-dangling-else
)

target_compile_options(
  solvers PRIVATE
  # "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  # "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${GLUCOSE_CLANG_WARNINGS}>"
)

set(MATHSAT_DIR ${CMAKE_SOURCE_DIR}/external/sat/mathsat5)

add_library(mathsat5 STATIC IMPORTED)
set_property(
  TARGET mathsat5
  PROPERTY IMPORTED_LOCATION ${MATHSAT_DIR}/lib/libmathsat.a
)

target_link_libraries(solvers PUBLIC mathsat5 gmpxx gmp)
