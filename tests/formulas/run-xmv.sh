#!/bin/bash

XMV=nuXmv

xmv() {
  output=$($XMV -source <(cat <<END
set input_file $1
go
build_boolean_model
bmc_setup
check_ltlspec_sbmc_inc -c
quit
END
  ) <<END
quit
END
)
  if (echo $output | grep "is false" > /dev/null 2>&1); then
    echo SAT
  else
    echo UNSAT
  fi
}

convert() {
  cat <<END
  MODULE main
  VAR
END

  cat $1 | grep -o '([a-zA-Z0-9]\{1,\})' | sort | uniq |
    sed 's/(\(.*\))/   \1 : boolean;/'

  formula=$(cat $1 | sed 's/=>/->/g;s/~/!/g;s/True/TRUE/g;s/False/FALSE/g')
  formula=$(echo $formula | sed 's/X/X /g;s/F/F /g;s/G/G /g')

  echo "LTLSPEC !($formula)"
}

if [ ! -n "$1" ]; then
  echo "Please specify a .pltl file to solve" 1>&2
  exit 1
fi

if [ ! -r "$1" ]; then
  echo "$0: $1: file is not readable"
  exit 1
fi

xmv <(convert $1)
