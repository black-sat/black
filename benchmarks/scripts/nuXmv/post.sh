#!/bin/bash

data=$(cat)

if echo $data | grep "is false" > /dev/null 2>&1; then
  echo "SAT"
elif echo $data | grep "is true" > /dev/null 2>&1; then
  echo "UNSAT"
fi
