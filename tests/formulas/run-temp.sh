#!/bin/bash

errors=0
crashes=0

die() {
  errors=$((errors+1))
}

cd $(git rev-parse --show-toplevel)

tests=./tests/formulas/tests.index

n=$(cat $tests | wc -l | sed 's/ //g')

i=1

for line in $(cat $tests); do
  file=$(echo $line | cut -d';' -f1)
  answer=$(echo $line | cut -d';' -f2)
  echo -n "Formula $i/$n: $file"
  result=$(./build/black -f ./tests/formulas/$file 2>/dev/null || die $file)
  if [ "$result" != "$answer" ];
  then
    errors=$((errors+1))
    wrong_files=$(echo "$wrong_files;$file")
    echo " - FAILED"
  else
    echo " - OK"
  fi
  i=$((i+1))
done

echo "Wrong replies: $errors"

if [ "$errors" -gt 0 ]; then
  echo "Wrong files:"
  IFS=';'
  for f in ${wrong_files:1}; do
    echo "- $f"
  done
fi
