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

set_property(TARGET asan APPEND PROPERTY INTERFACE_COMPILE_OPTIONS "-fsanitize=address")


find_library(
  UBSAN_LIBRARY
  NAMES libclang_rt.ubsan_osx_dynamic.dylib
  PATHS "${LLVM_PATH}/lib/clang/${LLVM_VERSION}/lib/darwin/")

add_library(ubsan SHARED IMPORTED GLOBAL)
set_property(TARGET ubsan PROPERTY IMPORTED_LOCATION ${UBSAN_LIBRARY})
set_property(TARGET ubsan APPEND PROPERTY INTERFACE_COMPILE_OPTIONS "-fsanitize=undefine")


add_library(sanitizer INTERFACE)
target_link_libraries(sanitizer INTERFACE asan)

#
# black library
#
set (
   LIB_SRC
   src/logic/lex.cpp
   src/logic/parser.cpp
   src/sat/mathsat.cpp
   src/solver/solver.cpp
 )

set (
 LIB_HEADERS
 include/black/support/common.hpp
 include/black/logic/alphabet.hpp
 include/black/logic/formula.hpp
 include/black/logic/lex.hpp
 include/black/logic/parser.hpp
 include/black/sat/glucose.hpp
 include/black/sat/mathsat.hpp
 include/black/solver/solver.hpp
)

# Here adding the headers is redundant, but makes them appear in
# project files made by IDE generators (Xcode, VS, ecc...)
add_library (black ${LIB_SRC} ${LIB_HEADERS})

target_link_libraries(black PUBLIC debug_assert solvers remotery)
target_link_libraries(black PRIVATE fmt)
target_include_directories(black PUBLIC include)
target_compile_features(black PUBLIC cxx_std_17)

#target_compile_options(black PUBLIC -O3 -g)

#target_link_libraries(black PUBLIC sanitizer)

target_compile_options(
  black PUBLIC
  "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
)
