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
# Unit testing with Catch2
#
add_library(catch INTERFACE)
target_include_directories(catch INTERFACE ${CMAKE_SOURCE_DIR}/external/catch/single_include/catch2)

set(
  UNIT_TESTS
  units/main.cpp
  units/common.cpp
  units/formula.cpp
  units/parser.cpp
)

add_executable(unit_tests ${UNIT_TESTS})
target_link_libraries(unit_tests black catch)

target_compile_options(
  unit_tests PUBLIC
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
)

add_test(NAME unit_tests COMMAND "${CMAKE_CURRENT_BINARY_DIR}/unit_tests")

message("-- Configuring tests")

file(STRINGS formulas/tests.index TESTS)

list(LENGTH TESTS TESTSLENGTH)
message("-- Found ${TESTSLENGTH} test formulas")

#
# Formulas test set
#
foreach(TEST IN LISTS TESTS)
  list(GET TEST 0 FILE)
  list(GET TEST 1 RESULT)

  add_test(
    NAME "black:${FILE}:${RESULT}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/formulas"
    COMMAND
      /bin/bash -c
      "$<TARGET_FILE:executable> -f '${FILE}' | grep '^${RESULT}'"
  )

  add_test(
    NAME "xmv:${FILE}:${RESULT}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/formulas"
    COMMAND
      /bin/bash -c
      "./run-xmv.sh '${FILE}' | grep '^${RESULT}'"
  )

  set_tests_properties("xmv:${FILE}:${RESULT}" PROPERTIES TIMEOUT 300)

endforeach()
