#!/bin/bash

trproblem=$(mktemp)

sed -e "s/True/true/g" -e "s/False/false/g" -e "s/~/not/g" -e "s/G/always/g" -e "s/F/sometime/g" -e "s/X/next/g" -e "s/U/until/g" \
    -e "s/=>/->/g"  < $1 > $trproblem

cat $trproblem

