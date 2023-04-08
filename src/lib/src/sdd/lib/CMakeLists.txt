#
# SDD++ - C++ wrapper library for libsdd 2.0
#
# (C) 2023 Nicola Gigante
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

cmake_minimum_required(VERSION 3.10...3.26)

set(
  SDD_SRC
  src/vtree_search/state.c
  src/vtree_search/auto.c
  src/vtree_search/search.c
  src/vtrees/static.c
  src/vtrees/compare.c
  src/vtrees/io.c
  src/vtrees/maps.c
  src/vtrees/moves.c
  src/vtrees/edit.c
  src/vtrees/vtree.c
  src/vtree_fragments/construction.c
  src/vtree_fragments/moves.c
  src/vtree_fragments/operations.c
  src/util.c
  src/vtree_operations/rollback.c
  src/vtree_operations/op_left_rotate.c
  src/vtree_operations/op_swap.c
  src/vtree_operations/split.c
  src/vtree_operations/limits.c
  src/vtree_operations/dissect.c
  src/vtree_operations/cartesian_product.c
  src/vtree_operations/op_right_rotate.c
  src/fnf/io.c
  src/verify.c
  src/basic/shadows.c
  src/basic/nodes.c
  src/basic/memory.c
  src/basic/sort.c
  src/basic/references.c
  src/basic/hash.c
  src/basic/count_and_size.c
  src/basic/replace.c
  src/basic/multiply.c
  src/basic/computed.c
  src/basic/partitions.c
  src/basic/gc.c
  src/manager/interface.c
  src/manager/variables.c
  src/manager/copy.c
  src/manager/stats.c
  src/manager/manager.c
  src/sdds/forall.c
  src/sdds/exists_multiple.c
  src/sdds/io.c
  src/sdds/wmc.c
  src/sdds/rename_vars.c
  src/sdds/apply.c
  src/sdds/copy.c
  src/sdds/exists_multiple_static.c
  src/sdds/bits.c
  src/sdds/size.c
  src/sdds/condition.c
  src/sdds/count.c
  src/sdds/cardinality.c
  src/sdds/model_count.c
  src/sdds/essential_vars.c
  src/sdds/exists.c
  src/version.c
)

add_library(sdd STATIC ${SDD_SRC})

option(SDD_ENABLE_FULL_DEBUG OFF)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_compile_definitions(sdd PRIVATE FULL_DEBUG=1)
else()
  target_compile_definitions(sdd PRIVATE FULL_DEBUG=0)
endif()

target_compile_definitions(sdd PRIVATE SDD_VERSION="2.0")

string(TIMESTAMP TODAY "%Y-%m-%d")
target_compile_definitions(sdd PRIVATE SDD_DATE="${TODAY}")

target_include_directories(sdd PRIVATE include)



