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
  -Wno-padded -Wno-weak-vtables -Wno-unknown-pragmas
  -Wno-exit-time-destructors -Wno-switch-enum
  -Wno-undefined-var-template
  -Wno-undefined-func-template
  -Wno-deprecated
  -Wno-old-style-cast -Wno-documentation
  -Wno-documentation-unknown-command
  -Wno-sign-conversion
  -Wno-global-constructors -Wno-extra-semi
  -Wno-unknown-warning-option
  # TODO: warnings ignored for fmt
  -Wno-undef -Wno-missing-noreturn
  -Wno-double-promotion
  # TODO: warnings ignored for clipp
  -Wno-reserved-id-macro
  -Wno-covered-switch-default
)

# GCC
set(
  GNU_WARNINGS -Wall -Wextra -pedantic -Werror
  -Wno-pragmas -Wno-unknown-pragmas -Wno-unused-but-set-parameter
)

# MSVC
set(MSVC_WARNINGS /wd4068 /wd4702 /W4 /WX)

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

