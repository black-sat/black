cmake_minimum_required(VERSION 3.8)

# set (
#   LIB_SRC
#   ...
# )
#
set (
 LIB_HEADERS
 include/black/logic/alphabet.hpp
 include/black/logic/formula.hpp
 include/black/support/common.hpp
)

# Here adding the headers is redundant, but makes them appear in
# project files made by IDE generators (Xcode, VS, ecc...)
# add_library (black STATIC ${LIB_SRC} ${LIB_HEADERS})

add_library(black INTERFACE)

target_link_libraries(black INTERFACE debug_assert fmt)
target_include_directories(black INTERFACE include)
target_compile_features(black INTERFACE cxx_std_17)

target_compile_options(
  black INTERFACE
  "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
  "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
)
