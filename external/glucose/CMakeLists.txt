cmake_minimum_required(VERSION 3.10)

project(glucose LANGUAGES C CXX)

find_package(ZLIB REQUIRED)

set(
  LIB_SRC
  # common
  core/Solver.cc
  utils/Options.cc
  utils/System.cc
  core/BoundedQueue.h
  core/Solver.h
  core/SolverStats.h
  core/Constants.h
  core/SolverTypes.h
  core/Dimacs.h
  mtl/IntTypes.h
  mtl/Map.h
  mtl/Vec.h
  mtl/Clone.h
  mtl/Alg.h
  mtl/XAlloc.h
  mtl/Heap.h
  mtl/Sort.h
  mtl/VecThreads.h
  mtl/Alloc.h
  mtl/Queue.h
  utils/Options.h
  utils/System.h
  utils/ParseUtils.h
  # Simp
  simp/SimpSolver.cc
  simp/SimpSolver.h
)

set(LIB_PARALEL_SRC
  # parallel
  parallel/SolverConfiguration.cc
  parallel/SharedCompanion.cc
  parallel/ClausesBuffer.h
  parallel/ParallelSolver.cc
  parallel/ClausesBuffer.cc
  parallel/SharedCompanion.h
  parallel/SolverCompanion.cc
  parallel/SolverConfiguration.h
  parallel/MultiSolvers.cc
  parallel/ParallelSolver.h
  parallel/SolverCompanion.h
  parallel/MultiSolvers.h
)

set(
  CLANG_WARNINGS
  -Wno-logical-op-parentheses
  -Wno-dangling-else
)

#
# Single-thread library and front-end
#

add_library(glucose STATIC ${SIMP_LIB_SRC} ${LIB_SRC})
target_link_libraries(glucose PUBLIC pthread)
set_target_properties(glucose PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(glucose PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(glucose PRIVATE -DNDEBUG)
target_compile_features(glucose PUBLIC cxx_std_11)
target_compile_options(
  glucose PRIVATE 
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:AppleClang>:${CLANG_WARNINGS}>"
)

add_library(glucose-parallel STATIC ${SIMP_LIB_SRC} ${LIB_SRC})
target_link_libraries(glucose-parallel PUBLIC glucose pthread)
set_target_properties(glucose-parallel PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(glucose PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(glucose PRIVATE -DNDEBUG)
target_compile_features(glucose PUBLIC cxx_std_11)
target_compile_options(
  glucose PRIVATE 
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:AppleClang>:${CLANG_WARNINGS}>"
)



add_executable(frontend-simp simp/Main.cc)
target_link_libraries(frontend-simp PRIVATE glucose ZLIB::ZLIB)
set_property(TARGET frontend-simp PROPERTY OUTPUT_NAME glucose)
target_compile_options(
  frontend-simp PRIVATE 
  -Wno-logical-op-parentheses
)

add_executable(frontend-syrup parallel/Main.cc)
target_link_libraries(frontend-syrup PRIVATE glucose glucose-parallel ZLIB::ZLIB)
set_property(TARGET frontend-syrup PROPERTY OUTPUT_NAME glucose-syrup)
target_compile_options(
  frontend-syrup PRIVATE 
  -Wno-logical-op-parentheses
)

