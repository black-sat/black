#!/bin/bash

if grep "is false" > /dev/null 2>&1; then
  echo "SAT"
else
  echo "UNSAT"
fi
