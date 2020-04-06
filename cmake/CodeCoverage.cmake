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

#
# Code to enable code coverage instrumentation
#

option(CODE_COVERAGE "Enable code coverage instrumentation" OFF)
option(LLVM_COV "Path to llvm-cov executable")

if(CODE_COVERAGE)
  message(STATUS "Checking for llvm-cov...")
  find_program(
    LLVM_COV_PATH llvm-cov  
    DOC "Path to llvm-cov command"
  )
  if(LLVM_COV_PATH)
    message(STATUS "Found llvm-cov: ${LLVM_COV_PATH}")
  else()
    message(WARNING "llvm-cov not found, skipping...")
  endif()
endif()

set(COV_CLANG_FLAGS -fprofile-instr-generate -fcoverage-mapping)

function(target_code_coverage TARGET)
  if(CODE_COVERAGE)
    target_compile_options(
      ${TARGET} 
      PRIVATE 
      "$<$<CXX_COMPILER_ID:Clang>:${COV_CLANG_FLAGS}>"
      "$<$<CXX_COMPILER_ID:AppleClang>:${COV_CLANG_FLAGS}>"
    )
    target_link_libraries(
      ${TARGET} PRIVATE
      "$<$<CXX_COMPILER_ID:Clang>:${COV_CLANG_FLAGS}>"
      "$<$<CXX_COMPILER_ID:AppleClang>:${COV_CLANG_FLAGS}>"
    )
  endif()
endfunction()