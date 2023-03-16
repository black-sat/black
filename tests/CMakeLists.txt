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

if(Catch2_FOUND)

  #
  # Unit testing with Catch2
  #
  set(
    UNIT_TESTS
    units/support/tribool.cpp
  )

  if(TARGET Catch2::Catch2WithMain)
    set(CATCH_TARGET Catch2::Catch2WithMain)
  else()
    set(CATCH_TARGET Catch2::Catch2)
    set(UNIT_TESTS ${UNIT_TESTS} units/main.cpp)
  endif()

  add_executable(unit_tests ${UNIT_TESTS})
  target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/units)
  target_link_libraries(unit_tests PRIVATE black ${CATCH_TARGET})
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
