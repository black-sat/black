cmake_minimum_required(VERSION 3.8)

#
# Workaround to link asan
# TODO: fix it decently
#

set(LLVM_PATH "/usr/local/opt/llvm/" CACHE STRING "Path of LLVM install tree")
set(LLVM_VERSION "7.0.1" CACHE STRING "LLVM version")

find_library(
  ASAN_LIBRARY
  NAMES libclang_rt.asan_osx_dynamic.dylib
  PATHS "${LLVM_PATH}/lib/clang/${LLVM_VERSION}/lib/darwin/")

add_library(asan SHARED IMPORTED GLOBAL)
set_property(TARGET asan PROPERTY IMPORTED_LOCATION ${ASAN_LIBRARY})
target_compile_options(asan INTERFACE "-fsanitize=address")


find_library(
  UBSAN_LIBRARY
  NAMES libclang_rt.ubsan_osx_dynamic.dylib
  PATHS "${LLVM_PATH}/lib/clang/${LLVM_VERSION}/lib/darwin/")

add_library(ubsan SHARED IMPORTED GLOBAL)
set_property(TARGET ubsan PROPERTY IMPORTED_LOCATION ${UBSAN_LIBRARY})
target_compile_options(ubsan INTERFACE "-fsanitize=undefined")


add_library(sanitizer INTERFACE)
target_link_libraries(sanitizer INTERFACE asan)
#target_link_libraries(sanitizer INTERFACE ubsan)

#
# black library
#
set (
   LIB_SRC
   src/logic/lex.cpp
   src/logic/parser.cpp
   src/solver/solver.cpp
   src/solver/mathsat.cpp
 )

set (
 LIB_HEADERS
 include/black/logic/alphabet.hpp
 include/black/logic/formula.hpp
 include/black/logic/lex.hpp
 include/black/logic/parser.hpp
 include/black/solver/solver.hpp
 include/black/support/common.hpp
)

# Here adding the headers is redundant, but makes them appear in
# project files made by IDE generators (Xcode, VS, ecc...)
add_library (black STATIC ${LIB_SRC} ${LIB_HEADERS})

target_link_libraries(black PUBLIC debug_assert solvers remotery)
target_link_libraries(black PRIVATE fmt)
target_include_directories(black PUBLIC include)
target_compile_features(black PUBLIC cxx_std_17)

target_compile_options(black PUBLIC -O3 -g)

#target_link_libraries(black PUBLIC sanitizer)

target_compile_options(
  black PUBLIC
  "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
)
