#!/bin/bash

# (C) 2020 Gabriele Venturato
# Script to filter out formulas from the benchmark formulas index which takes
# more than 3 seconds. In this way they can be used for rapid tests after
# compilation.

# --------------------------------- Utilities ----------------------------------
report() {
  echo "$@" >&2
}

die() {
  report "$@"
  exit 1
}

help() {
  cat <<HELP
Usage: ./$(basename "$0") [options]

Filter out formulas from benchmark formulas index which take more than 3 seconds
to execute.

Options:
  --formulasdir       Set the path to the benchmark formulas directory in which
                      there must be the index file.

  -e | --exec         Specify the BLACK executable file path. By default some
                      standard paths are checked.

  --args              Arguments to pass to BLACK.

  -f | --filter       Apply a grep filter to the index list of formulas.

  -h | --help         Print this help.

Examples:
  To check and filter out only acacia family formulas:
    ./$(basename "$0") --filter acacia/
HELP
}

errhelp() {
  help >&2
}

# ----------------------------------- Main -------------------------------------
FORMULAS_DIR=../../benchmarks/formulas

# Try to find black executable
BLACK=./black
if [[ ! -x "$BLACK" ]]; then
  BLACK=../../build/black
fi
if [[ ! -x "$BLACK" ]]; then
  BLACK=../../cmake-build-release/black
fi

# Optional parameters
POSITIONAL=()
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    --formulasdir)
      FORMULAS_DIR="$2"
      shift
      shift
      ;;
    -e|--exec)
      BLACK="$2"
      shift
      shift
      ;;
    --args)
      ARGS="$2"
      shift
      shift
      ;;
    -f|--filter)
      FILTER="$2"
      shift
      shift
      ;;
    -h|--help)
      help
      exit 0
      ;;
    *) # unknown option
      POSITIONAL+=("$1") # save it in an array for later
      shift
      ;;
  esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

if [[ $# -ne 0 ]]; then
  errhelp
  exit 1
fi

# Check if FORMULAS_DIR exists
if [[ ! -f "$FORMULAS_DIR/index" ]]; then
  die "Error: directory $FORMULAS_DIR is not the benchmark formulas one."
fi

# Check if BLACK executable exists and is executable
if [[ ! -x "$BLACK" ]]; then
  die "Error: $BLACK is not executable or does not exists."
fi

# Filtering
grep "$FILTER" "$FORMULAS_DIR/index" | while IFS= read -r formula; do
  if res=$(timeout -s KILL 3s \
            "$BLACK" $ARGS "$FORMULAS_DIR/$formula" 2>/dev/null)
  then
    echo "$formula;$res"
  fi
done