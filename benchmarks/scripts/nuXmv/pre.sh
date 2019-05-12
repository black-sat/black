#!/bin/bash

cat <<END
MODULE main
VAR
END

cat $1 | grep -o '([a-zA-Z0-9]\{1,\})' | sort | uniq |
  sed 's/(\(.*\))/   \1 : boolean;/'

formula=$(cat $1 | sed 's/=>/->/g;s/~/!/g;s/True/TRUE/g;s/False/FALSE/g')

echo "LTLSPEC !($formula)"
