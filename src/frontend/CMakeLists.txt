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

find_package(nlohmann_json 3.5.0 REQUIRED)

#
# Main black executable
#
set(
  FRONTEND_SRC
  src/main.cpp
  src/cli.cpp
  src/solve.cpp
  src/dimacs.cpp
  src/tracecheck.cpp
  src/support.cpp
)

set(
  FRONTEND_HEADERS
  include/black/frontend/cli.hpp
  include/black/frontend/io.hpp
  include/black/frontend/support.hpp
)

add_executable(frontend ${FRONTEND_SRC} ${FRONTEND_HEADERS})

target_compile_features(frontend PRIVATE cxx_std_20)
target_include_directories(frontend PRIVATE include)
target_link_libraries(frontend PRIVATE black fmt::fmt clipp::clipp)
target_link_libraries(frontend PRIVATE nlohmann_json::nlohmann_json)
target_enable_warnings(frontend)
target_code_coverage(frontend)
add_sanitizers(frontend)

if(WIN32)
	target_link_options(frontend PRIVATE /stack:8388608)
endif()

set_property(TARGET frontend PROPERTY OUTPUT_NAME black)
set_property(TARGET frontend PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

install(TARGETS frontend RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
