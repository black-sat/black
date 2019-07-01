#!/bin/bash

data=$(cat)

if echo $data | grep "unsatisfiable" > /dev/null 2>&1; then
  echo "UNSAT"
elif echo $data | grep "satisfiable" > /dev/null 2>&1; then
  echo "SAT"
fi
