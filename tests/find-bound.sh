#!/bin/bash 

set -o pipefail

die() {
  echo $1 >&2
  exit 1
}

if [ -z "$1" ] || [ -z "$2" ]; then
  die "usage: $0 <domain> <pattern> [<timeout>]"
fi

DOMAIN=$1
PATTERN=$2
TIMEOUT=300s

if [ -n "$3" ]; then
  TIMEOUT=$3
fi

BLACK="$(git rev-parse --show-toplevel)/build/black"

oldtime=0.00
i=2
while true; do
  file=$(echo $PATTERN | sed "s/N/$i/g")
  
  if [ ! -e "$file" ]; then
    die "$0: $file: No such file or directory"
  fi
  
  oldtime=$time
  time=$({ time -p timeout $TIMEOUT $BLACK solve -s -d $DOMAIN $file ; } 2>&1 | grep real | cut -d ' ' -f 2)

  if [ "$?" -ne 0 ]; then
    echo $PATTERN
    echo "N = $((i - 1)) - $oldtime"
    exit 0
  fi

  echo "i: $i - time: $time"

  i=$((i + 1))
done