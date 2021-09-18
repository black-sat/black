#!/bin/bash

cd solvers/ltl2sat/bin
cat $1 | java -jar LTL2SAT.jar -t $((TIMEOUT+100))
