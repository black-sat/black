#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2019 - 2020 Nicola Gigante
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

find_package(MathSAT REQUIRED)
find_package(Z3)
find_package(cryptominisat5 QUIET)

message(STATUS "MathSAT solver found! Enabling the MathSAT backend...")

if(Z3_FOUND)
  message(STATUS "Z3 solver found! Enabling the Z3 SAT backend...")
else()
  message(STATUS "Z3 solver not found. Disabling the Z3 SAT backend...")
endif()

if(cryptominisat5_FOUND)
  message(STATUS 
    "CryptoMiniSAT solver found! Enabling the CMSAT SAT backend...")
else()
  message(STATUS 
    "CryptoMiniSAT solver not found. Disabling the CMSAT SAT backend...")
endif()

#
# black library
#
set (
   LIB_SRC
   src/logic/formula.cpp
   src/logic/lex.cpp
   src/logic/parser.cpp
   src/sat/sat.cpp
   src/sat/cnf.cpp
   src/sat/backends/mathsat.cpp
   src/solver/solver.cpp
   src/logic/past_remover.cpp
   )
   
if(Z3_FOUND)
  set(LIB_SRC ${LIB_SRC} 
    src/sat/backends/z3.cpp include/black/sat/backends/z3.hpp)
endif()

if(cryptominisat5_FOUND)
  set(LIB_SRC ${LIB_SRC} 
    src/sat/backends/cmsat.cpp include/black/sat/backends/cmsat.hpp)
endif()

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
  include/black/solver/solver.hpp
        include/black/logic/past_remover.hpp)

#
# Main black library. The `internal` trick is needed to make sure global 
# constructors are correctly called when the library is statically linked
#
add_library (black ${LIB_SRC} ${LIB_HEADERS})

target_link_libraries(black PRIVATE MathSAT fmt)
target_link_libraries(black PUBLIC tsl::hopscotch_map)
target_include_directories(black PUBLIC include)
target_compile_features(black PUBLIC cxx_std_17)
if(Z3_FOUND)
  target_link_libraries(black PRIVATE Z3)
endif()
if(cryptominisat5_FOUND)
  target_link_libraries(black PRIVATE cryptominisat5)
endif()
target_enable_warnings(black)
target_code_coverage(black)
add_sanitizers(black)

##
## Z3 SMT solver
##
