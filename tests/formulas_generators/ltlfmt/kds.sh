#!/bin/bash

err() {
  echo "$@"
}

die() {
  err "$@"
  exit 1
}

usage() {
  echo "$@"
  echo "usage: kds.sh <N> <T>"
  echo " - <N>: maximum number of iterations"
  echo " - <T>: timeout in seconds"
  exit 1
}

run() {
  T=$1
  
}

run() {
  T=$1
  shift

  result=$(timeout $T \time ./kds "$@" 2>&1 1>\dev\null | awk '{print $1}')
  if [ -z "$result" ]; then
    echo "timeout"
  else
    echo $result
  fi
}

main() {
  if [ -z "$1" ]; then
    usage "Please specify the number of iterations"
  fi

  if [ -z "$2" ]; then
    usage "Please specify the timeout"
  fi

  N=$1
  T=$2

  cd "$(git rev-parse --show-toplevel)/build/tests"

  echo Running with N = $1 and T = $2... >&2

  echo "N P1s P1l P2s P2l"

  for n in $(seq 5 5 $N); do
    echo -n $n
    for P in 1 2; do
      for S in s l; do
        echo -n "n = $n/$N, P = $P, S = $S, ..." 1>&2
        result=$(run $T $P $S $n)
        echo " $result" >&2
        echo -n " $result"
      done
    done
    echo
  done 
  

}

main "$@"
