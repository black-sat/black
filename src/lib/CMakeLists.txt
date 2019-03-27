cmake_minimum_required(VERSION 3.8)

set (
   LIB_SRC
   src/logic/lex.cpp
   src/logic/parser.cpp
   src/solver/solver.cpp
 )

set (
 LIB_HEADERS
 include/black/logic/alphabet.hpp
 include/black/logic/formula.hpp
 include/black/logic/lex.hpp
 include/black/logic/parser.hpp
 include/black/solver/solver.hpp
 include/black/support/common.hpp
)

# Here adding the headers is redundant, but makes them appear in
# project files made by IDE generators (Xcode, VS, ecc...)
add_library (black STATIC ${LIB_SRC} ${LIB_HEADERS})

target_link_libraries(black PUBLIC debug_assert)
target_link_libraries(black PRIVATE fmt solvers)
target_include_directories(black PUBLIC include)
target_compile_features(black PUBLIC cxx_std_17)

target_compile_options(black PUBLIC -g)

target_compile_options(
  black PUBLIC
  "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
)
