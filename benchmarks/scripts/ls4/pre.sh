#!/bin/bash

trproblem=$(mktemp)

sed -e "s/True/(AAA | not AAA)/g" -e "s/False/(AAA \& not AAA)/g" -e "s/~/not/g" -e "s/G/always/g" -e "s/F/sometime/g" -e "s/X/next/g" -e "s/U/until/g" \
    -e "s/=>/->/g"  < $1 > $trproblem

echo . >> $trproblem

cat $trproblem

