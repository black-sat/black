#!/bin/bash

if [ "" = "$1" ]; then
  echo "Please specify a .pltl file to convert" 1>&2
  exit 1
fi

echo MODULE main
echo VAR
echo '  ' True : boolean\;
echo '  ' False : boolean\;

cat $1 | grep -o '([a-zA-Z0-9]\{1,\})' | sort | uniq | sed 's/(\(.*\))/   \1 : boolean;/'

echo "LTLSPEC !(G(True) & G(!False) & ($(cat $1 | sed 's/=>/->/g;s/~/!/g')))"
