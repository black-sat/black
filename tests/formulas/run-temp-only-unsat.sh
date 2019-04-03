#!/bin/bash

errors=0
crashes=0

die() {
  errors=$((errors+1))
}

cd $(git rev-parse --show-toplevel)

tests=./tests/formulas/tests_unsat.index

n=$(cat $tests | wc -l | sed 's/ //g')

i=1

for line in $(cat $tests); do
  file=$(echo $line | cut -d';' -f1)
  answer=$(echo $line | cut -d';' -f2)
  echo "Formula $i/$n: $file"
  echo "Answer: $answer"
  result=$(./build/black -f ./tests/formulas/$file 2>/dev/null )
  echo "Result: $result"
  if [ "$result" != "$answer" ];
  then
    errors=$((errors+1))
    wrong_files=$(echo "$wrong_files;$file")
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
