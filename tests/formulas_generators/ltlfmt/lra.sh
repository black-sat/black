#!/bin/bash

#
# BLACK - Bounded Ltl sAtisfiability ChecKer
#
# (C) 2021 Luca Geatti
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

help() {
 cat <<HELP
Usage ./$(basename "$0")

Generates formulas for LRA theory of categories 1 for N ∈ {10,10^10}
and of category 2 for N ∈ {2,17}
HELP
}

CATEGORY="$1"
BIN="../../../build/tests/lra"
OUTDIR="../../../benchmarks/formulas/ltlf-modulo-theory/LRA/scalable_$CATEGORY"

re='^[0-9]+$'
if ! [[ $CATEGORY =~ $re ]] ; then
   echo "ERROR: the <category> parameter must be a number" >&2; exit 1
fi

if [[ $CATEGORY == "1" ]] ; then
  for (( i=1; i<=200; i=i+1 ))
  do
    $BIN $CATEGORY $i > "$OUTDIR/scalable-$CATEGORY-$i.ltlfmt"
    echo "Created benchmark #$i for Category $CATEGORY and Theory LRA"
  done
fi

if [[ $CATEGORY == "2" ]] ; then
  for (( i=2; i<=200; i=i+1 ))
  do
    $BIN $CATEGORY $i > "$OUTDIR/scalable-$CATEGORY-$i.ltlfmt"
    echo "Created benchmark #$i for Category $CATEGORY and Theory LRA"
  done
fi
