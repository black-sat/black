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

set(BLACK_TESTS_SAT_BACKEND "z3" CACHE STRING "SAT backend used in tests")

#
# Formulas generators
#
add_executable(crscounter_generator
        formulas_generators/crscounter/crscounter_generator.cpp)
target_link_libraries(crscounter_generator PRIVATE black)

add_executable(random_formulas_generator
        formulas_generators/random_formulas/random_formulas_generator.cpp)
target_link_libraries(random_formulas_generator PRIVATE black)

add_custom_target(ltlfmt
  DEPENDS lia)

add_executable(lia
        formulas_generators/ltlfmt/lia.cpp)
target_link_libraries(lia PRIVATE black fmt::fmt)

set_target_properties(
  crscounter_generator random_formulas_generator ltlfmt lia
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
    units/formula.cpp
    units/term.cpp
    units/main.cpp
    units/match.cpp
    units/parser.cpp
    units/solver.cpp
    units/past_remover.cpp
    units/support.cpp
    units/sat.cpp
  )

  add_executable(unit_tests ${UNIT_TESTS})
  target_link_libraries(unit_tests PRIVATE black Catch2::Catch2)
  target_enable_warnings(unit_tests)
  target_code_coverage(unit_tests)
  add_sanitizers(unit_tests)

  add_test(
    NAME unit_tests 
    COMMAND "$<TARGET_FILE:unit_tests>"
  )

  set_property(
    TARGET unit_tests PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  )

  message(STATUS "Unit tests enabled")
else()
  message(STATUS
    "Catch2 missing. Unit tests disabled (correctness test suite still enabled)"
  )
endif()

#
# CLI tests
#
if(NOT WIN32)
  add_test(
    NAME "CLI"
    COMMAND ${CMAKE_SOURCE_DIR}/tests/cli-tests.sh 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endif()

#
# Formulas test set
#
if(ENABLE_FORMULAS_TESTS)
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
    set(FINITE_OPT "")
    string(FIND "${FILE}" "finite_models" FOUND)
    if(FOUND GREATER_EQUAL 0)
      set(FINITE_OPT "--finite")
    endif()
    
    set(SHELL_CMD /bin/sh -c)
    if(WIN32)
      set(SHELL_CMD powershell -Command)
    endif()

    add_test(
      NAME "${TEST_NAME}"
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/benchmarks/formulas"
      COMMAND
        ${SHELL_CMD}
        "$<TARGET_FILE:frontend> solve \
        -m -o json ${FINITE_OPT} -B ${BLACK_TESTS_SAT_BACKEND} '${FILE}' | \
        $<TARGET_FILE:frontend> check -t - ${FINITE_OPT} -e ${RESULT} '${FILE}'"
    )

    if(BLACK_TEST_TIMEOUT)
      set_tests_properties(
        "${TEST_NAME}" PROPERTIES TIMEOUT ${BLACK_TEST_TIMEOUT}
      )
    endif()

  endforeach()
endif()
