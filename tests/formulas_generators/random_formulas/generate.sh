#!/bin/bash

# (C) 2020 Gabriele Venturato
# Script to generate LTL+Past random formulas through the C++ generator.

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
Usage: ./$(basename "$0") <num> <dim> [options]

Generates <num> random formulas of dimension <dim>.

Options:
  -e | --exec       To specify the executable file path, if it is not in the
                    current directory.

  -o | --out        To specify an output directory. Default is the current one.

  --i-min           Start generating formulas from a minimum index. Useful to
                    not override already generated formulas. Default is 1.

  --ltl             To exclude past operators.
  
  --sltl            To generate Standpoint LTL formulas

  -h | --help       Print this help.
HELP
}

errhelp() {
  help >&2
}

# ----------------------------------- Main -------------------------------------
OUT_DIR=.
LTL=""
EXEC=random_formulas_generator
I_MIN=1
EXT=pltl

# Optional parameters
POSITIONAL=()
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -e|--exec)
      EXEC="$2"
      shift
      shift
      ;;
    -o|--out)
      OUT_DIR="$2"
      shift
      shift
      ;;
    --i-min)
      I_MIN="$2"
      shift
      shift
      ;;
    --ltl)
      LTL="--ltl"
      shift
      ;;
    --sltl)
      LTL="--sltl"
      EXT="sltl"
      shift
      ;;
    -h|--help)
      help
      exit 0
      ;;
    *)    # unknown option
      POSITIONAL+=("$1") # save it in an array for later
      shift
      ;;
  esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

# Mandatory parameters
if [[ $# -ne 2 ]]; then
  errhelp
  exit 1
fi

NUM=$1
DIM=$2

# Check if OUT_DIR exists
if [[ ! -d "$OUT_DIR" ]]; then
  die "Directory $OUT_DIR does not exists."
fi

# Check if EXEC exists and is executable
if [[ ! -x "$EXEC" ]]; then
  die "File $EXEC is not executable or does not exists."
fi

# Generation
formulas=$($EXEC --num "$NUM" --dim "$DIM" $LTL)

i=$I_MIN
while IFS= read -r line; do
  echo "$line" > "$OUT_DIR/random_formulas_dim$DIM""_$i.$EXT"
  i=$((i+1))
done <<< "$formulas"

