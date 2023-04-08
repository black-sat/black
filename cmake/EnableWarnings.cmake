#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2020 Nicola Gigante
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
# Curated list of warnings that we want to enable or disable on the 
# different compilers.
#
# To add them to a target, this file provides the macro target_enable_warnings()
#

# Clang
set(
  CLANG_WARNINGS -Weverything -pedantic -Werror
  -Wno-c++98-compat -Wno-c++98-compat-pedantic
  -Wno-c++98-c++11-c++14-compat
  -Wno-padded 
  -Wno-weak-vtables 
  -Wno-unknown-pragmas
  -Wno-exit-time-destructors 
  -Wno-return-std-move-in-c++11
  # -Wno-switch-enum
  -Wno-undefined-var-template
  -Wno-undefined-func-template
  # -Wno-deprecated
  -Wno-old-style-cast 
  -Wno-documentation
  -Wno-documentation-unknown-command
  # -Wno-sign-conversion
  -Wno-global-constructors 
  # -Wno-extra-semi
  -Wno-unknown-warning-option
  -Wno-poison-system-directories # fix for macOS Big Sur
  -Wno-float-equal
  -Wno-extra-semi-stmt
  -Wno-missing-braces
  -Wno-zero-as-null-pointer-constant
  -Wno-unsafe-buffer-usage
  -Wno-newline-eof
)

# GCC
set(
  GNU_WARNINGS -Wall -Wextra -pedantic -Werror
  -Wno-pragmas -Wno-unknown-pragmas -Wno-unused-but-set-parameter
)

# MSVC
set(
  MSVC_WARNINGS 
  /wd4068 /wd4702 /wd4127 /wd4459 /wd4573 /wd4251 /wd4244
)

#
# target_enable_warnings()
#
macro(target_enable_warnings TARGET)
  target_compile_options(
    ${TARGET} PUBLIC
    "$<$<CXX_COMPILER_ID:MSVC>:${MSVC_WARNINGS}>"
    "$<$<CXX_COMPILER_ID:GNU>:${GNU_WARNINGS}>"
    "$<$<CXX_COMPILER_ID:Clang>:${CLANG_WARNINGS}>"
    "$<$<CXX_COMPILER_ID:AppleClang>:${CLANG_WARNINGS}>"
  )
endmacro()

#
# Marks all interface headers of a target as system headers, to ignore warnings
#
function(target_mark_as_system TARGET)
  get_target_property(
    TARGET_INCLUDE_DIR ${TARGET} INTERFACE_INCLUDE_DIRECTORIES
  )
  set_target_properties(
    ${TARGET} PROPERTIES 
    INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${TARGET_INCLUDE_DIR}"
  )
endfunction()