#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2022 Nicola Gigante
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

option(BLACK_ENABLE_PYTHON_BINDINGS "Enable Python bindings" OFF)

if(BLACK_ENABLE_PYTHON_BINDINGS)

  find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
  find_package(pybind11 REQUIRED)

  message(STATUS "Python sitelib: ${Python3_SITELIB}")

  if(Python3_FOUND AND pybind11_FOUND)

    message(STATUS "Python and PyBind11 found! Enabling Python bindings...")

    pybind11_add_module(
      bindings 
      src/main.cpp 
      src/support.cpp
      src/logic.cpp
    )

    target_include_directories(
      bindings PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(bindings PUBLIC black)
    set_target_properties(bindings PROPERTIES OUTPUT_NAME black_sat)
  
  endif()
    
endif()

