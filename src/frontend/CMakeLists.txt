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
# Main black executable
#
set(
  FRONTEND_SRC
  src/main.cpp
  src/cli.cpp
)

set(
  FRONTEND_HEADERS
  include/black/frontend/cli.hpp
  include/black/frontend/support.hpp
)

add_executable(frontend ${FRONTEND_SRC} ${FRONTEND_HEADERS})

target_compile_features(frontend PRIVATE cxx_std_17)
target_include_directories(frontend PRIVATE include)
target_link_libraries(frontend PRIVATE black solvers)
target_link_libraries(frontend PRIVATE debug_assert fmt clipp::clipp solvers)
target_compile_options(
  frontend PRIVATE
  "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
)

if(STATIC_BUILD)
  target_link_libraries(frontend PRIVATE "-static -pthread")
endif()


set_property(TARGET frontend PROPERTY OUTPUT_NAME black)
set_property(TARGET frontend PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

install(TARGETS frontend RUNTIME DESTINATION bin)
