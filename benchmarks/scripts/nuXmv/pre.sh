#!/bin/bash

cat <<END
MODULE main
VAR
END

cat $1 | grep -o '[a-zA-Z0-9_]\{1,\}' | sort | uniq |
  grep -vw "X" | grep -vw "F" | grep -vw "G" | grep -vw "U" | grep -vw "R" | 
  grep -vw "Y" | grep -vw "P" | grep -vw "H" | grep -vw "S" | grep -vw "T" | 
  sed 's/\(.*\)/   \1 : boolean;/'

formula=$(cat $1 |
  sed 's/=>/->/g;s/~/!/g;s/True/TRUE/g;s/False/FALSE/g;s/ R / V /g')

echo "LTLSPEC !($formula)"
