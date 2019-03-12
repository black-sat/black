cmake_minimum_required(VERSION 3.8)

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
