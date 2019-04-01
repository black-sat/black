#!/bin/bash

errors=0

die() {
  echo "Wrong reply for $1"
  #exit 1
  errors=$((errors+1))
}

cd $(git rev-parse --show-toplevel)

# SAT formulae only
#sat_tests=$(cat ./tests/formulas/tests.index | grep -v UNSAT | cut -d';' -f 1)
# UNSAT formulae only
sat_tests=$(cat ./tests/formulas/tests.index | grep -v ';SAT' | cut -d';' -f 1)
# All the formulae in the test set
#sat_tests=$(cat ./tests/formulas/tests.index | cut -d';' -f 1)

n=$(wc -l <(echo $sat_tests) | awk '{print $1}')

i=1

for f in $sat_tests; do
  echo "Formula $i/$n: $f"
  ./build/black -f ./tests/formulas/$f || die $f
  i=$((i+1))
done

echo "Wrong replies: $errors"
