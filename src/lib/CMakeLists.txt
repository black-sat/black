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
# black library
#
set (
   LIB_SRC
   src/logic/lex.cpp
   src/logic/parser.cpp
   src/sat/sat.cpp
   src/sat/mathsat.cpp
   src/sat/z3.cpp
   src/solver/solver.cpp
 )

set (
  LIB_HEADERS
  include/black/internal/formula/base.hpp
  include/black/internal/formula/impl.hpp
  include/black/internal/formula/match.hpp
  include/black/internal/alphabet_impl.hpp
  include/black/support/common.hpp
  include/black/logic/alphabet.hpp
  include/black/logic/formula.hpp
  include/black/logic/lex.hpp
  include/black/logic/parser.hpp
  include/black/sat/mathsat.hpp
  include/black/solver/solver.hpp
)

# Here adding the headers is redundant, but makes them appear in
# project files made by IDE generators (Xcode, VS, ecc...)
add_library (black ${LIB_SRC} ${LIB_HEADERS})

target_link_libraries(black PUBLIC solvers)
target_link_libraries(black PRIVATE fmt tsl::hopscotch_map)
target_include_directories(black PUBLIC include)
target_compile_features(black PUBLIC cxx_std_17)
target_enable_warnings(black)
target_code_coverage(black)
add_sanitizers(black)