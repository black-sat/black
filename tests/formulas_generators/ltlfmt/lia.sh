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
Usage ./$(basename "$0") <category> <startindex> <endindex>

Generates formulas for LIA theory of <category> for N ∈ {<startindex> .. <endindex>}
HELP
}

CATEGORY="$1"
START="$2"
END="$3"
BIN="../../../build/tests/lia"
OUTDIR="../../../benchmarks/formulas/ltlf-modulo-theory/LIA/scalable_$CATEGORY"

re='^[0-9]+$'
if ! [[ $CATEGORY =~ $re ]] ; then
   echo "ERROR: the <category> parameter must be a number" >&2; exit 1
fi
if ! [[ $START =~ $re ]] ; then
   echo "ERROR: the <startindex> parameter must be a number" >&2; exit 1
fi
if ! [[ $END =~ $re ]] ; then
   echo "ERROR: the <endindex> parameter must be a number" >&2; exit 1
fi

for (( i=$START; i<=$END; i=i+1 ))
do
  $BIN $CATEGORY $i > "$OUTDIR/scalable-$CATEGORY-$i.ltlfmt"
  echo "Created benchmark #$i for Category $CATEGORY and Theory LIA"
done

