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

find_package(Catch2 QUIET)

option(
  ENABLE_TOOLS_TESTS "Enable tests of other tools in the benchmark suite" OFF
)

set(BLACK_TESTS_SAT_BACKEND "z3" CACHE STRING "SAT backend used in tests")

#
# Formulas generators
#
add_executable(crscounter_generator
        formulas_generators/crscounter/crscounter_generator.cpp)
target_link_library_with_global_ctors(crscounter_generator PRIVATE black)

add_executable(random_formulas_generator
        formulas_generators/random_formulas/random_formulas_generator.cpp)
target_link_library_with_global_ctors(random_formulas_generator PRIVATE black)

set_target_properties(
  crscounter_generator random_formulas_generator 
  PROPERTIES 
  EXCLUDE_FROM_ALL TRUE
)


if(Catch2_FOUND)

  #
  # Unit testing with Catch2
  #
  set(
    UNIT_TESTS
    units/cnf.cpp
    units/hash.cpp
    units/formula.cpp
    units/main.cpp
    units/match.cpp
    units/parser.cpp
    units/solver.cpp
    units/past_remover.cpp
  )

  add_executable(unit_tests ${UNIT_TESTS})
  target_link_library_with_global_ctors(unit_tests PRIVATE black)
  target_link_libraries(unit_tests PRIVATE Catch2::Catch2)
  target_enable_warnings(unit_tests)
  target_code_coverage(unit_tests)
  add_sanitizers(unit_tests)

  add_test(NAME unit_tests COMMAND "${CMAKE_CURRENT_BINARY_DIR}/unit_tests")

  message(STATUS "Unit tests enabled")
else()
  message(STATUS
    "Catch2 missing. Unit tests disabled (correctness test suite still enabled)"
  )
endif()

#
# Formulas test set
#
message(STATUS "Configuring test suite...")

file(STRINGS formulas/tests.index TESTS)

list(LENGTH TESTS TESTSLENGTH)
message(STATUS "Found ${TESTSLENGTH} test formulas")

option(BLACK_TEST_TIMEOUT "Set a timeout for tests" OFF)
set(TEST_TIMEOUT_SECS 3 CACHE STRING "Seconds of timeout for tests")

if(BLACK_TEST_TIMEOUT)
  message(STATUS "Tests timeout set to ${TEST_TIMEOUT_SECS} seconds")
endif()

foreach(TEST IN LISTS TESTS)
  list(GET TEST 0 FILE)
  list(GET TEST 1 RESULT)

  set(TEST_NAME "black:${FILE}:${RESULT}")
  add_test(
    NAME "${TEST_NAME}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/benchmarks/formulas"
    COMMAND
      /bin/bash -c
      "$<TARGET_FILE:frontend> \
      --sat-backend ${BLACK_TESTS_SAT_BACKEND} '${FILE}' | grep '^${RESULT}'"
  )

  if(BLACK_TEST_TIMEOUT)
    set_tests_properties(
      "${TEST_NAME}" PROPERTIES TIMEOUT ${BLACK_TEST_TIMEOUT}
    )
  endif()

  if(ENABLE_TOOLS_TESTS)
    foreach(SOLVER IN ITEMS nuXmv/klive nuXmv/sbmc aalta/v2)
      set(TEST_NAME "${SOLVER}:${FILE}:${RESULT}")
      add_test(
        NAME "${TEST_NAME}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/benchmarks"
        COMMAND
          /bin/bash -c
          "./run.sh run ${SOLVER} '${FILE}' | grep '^${RESULT}'"
      )

      set_tests_properties("${TEST_NAME}"
        PROPERTIES TIMEOUT ${TEST_TIMEOUT_SECS}
      )
    endforeach()
  endif()

endforeach()
