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
# Macro to link a static library with functioning global constructors
#
# see https://github.com/bioinformatics-centre/kaiju/issues/30
#

macro(target_link_library_with_global_ctors TARGET MODE LIBRARY)
  set(CLANG_GLOBAL_CTORS_FLAGS 
    "-Wl,-force_load" ${LIBRARY})
  set(GNU_GLOBAL_CTORS_FLAGS 
    "-Wl,-whole-archive" ${LIBRARY} "-Wl,-no-whole-archive")
  
  target_link_libraries(${TARGET} ${MODE} 
    "$<$<CXX_COMPILER_ID:Clang>:${CLANG_GLOBAL_CTORS_FLAGS}>"
    "$<$<CXX_COMPILER_ID:AppleClang>:${CLANG_GLOBAL_CTORS_FLAGS}>"
    "$<$<CXX_COMPILER_ID:GNU>:${GNU_GLOBAL_CTORS_FLAGS}>"
  )
endmacro()