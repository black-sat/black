#!/bin/bash

# (C) 2020 Gabriele Venturato
# Script to generate LTL+Past random formulas through the C++ generator.

help() {
  cat <<HELP
Usage: ./$(basename $0) <num> <dim> [options]

Generates <num> random formulas of dimension <dim>.

Options:
  -e | --exec                     To specify the executable file, if it is not
                                  in the current directory.

  -o | --out                      To specify an output directory. Default is the
                                  current one.

  --ltl                           To exclude past operators.
HELP
}

OUT_DIR=.
LTL=""
EXEC=random_formulas_generator

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
    --ltl)
      LTL="--ltl"
      shift
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
  help
  exit 1
fi

NUM=$1
DIM=$2

# Check if OUT_DIR exists
if [[ ! -d "$OUT_DIR" ]]; then
  echo "Directory $OUT_DIR does not exists."
  exit 1
fi

# Check if EXEC exists and is executable
if [[ ! -x "$EXEC" ]]; then
  echo "File $EXEC is not executable or does not exists."
  exit 1
fi

# Generation
formulas=$($EXEC --num $NUM --dim $DIM $LTL)

i=1
while IFS= read -r line; do
  echo "$line" > "$OUT_DIR/random_formulas_dim$DIM""_$i.pltl"
  i=$((i+1))
done <<< "$formulas"

