#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2024 Nicola Gigante
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

cmake_minimum_required(VERSION 3.10)

project(
  examples 
  VERSION 0.0.1
  LANGUAGES C CXX
  DESCRIPTION "Usage examples of BLACK's API"
)

find_package(black)

add_executable(quadratic basics/quadratic.cpp)
target_link_libraries(quadratic black::black)

add_executable(factorial basics/factorial.cpp)
target_link_libraries(factorial black::black)

add_executable(formulas logic/formulas.cpp)
target_link_libraries(formulas black::black)

add_executable(tutorial API_test/tutorial.cpp)
target_link_libraries(tutorial black::black)

add_executable(1_future_to_past API_test/1_future_to_past.cpp)
target_link_libraries(1_future_to_past black::black)

add_executable(2_to_nnf API_test/2_to_nnf.cpp)
target_link_libraries(2_to_nnf black::black)

add_executable(3_free_vars_counter API_test/3_free_vars_counter.cpp)
target_link_libraries(3_free_vars_counter black::black)