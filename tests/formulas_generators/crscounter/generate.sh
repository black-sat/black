#!/bin/bash

# (C) 2020 Gabriele Venturato
# Script to generate LTL+Past parametrized properties through the C++ generator.

help() {
  cat <<HELP
Usage: ./$(basename $0) <N> <i_max> [options]

Generates formulas with N assigned, and i from 0 (or i_min) to i_max

Options:
  -e | --exec                     To specify the executable file, if it is not
                                  in the current directory.

  -o | --out                      To specify an output directory. Default is the
                                  current one.

  --i_min                         To start generation from a specific i value.
                                  Default is 0.

  --i_step                        Set a step for i increment. Default is 1.

  --N_template                    Use this instead of N value to set filename.

  --next                          Use counter based on X instead of Y operators.
HELP
}

OUT_DIR=.
I_MIN=0
I_STEP=1
NEXT=""
EXEC=crscounter_generator

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
    --i_min)
      I_MIN="$2"
      shift
      shift
      ;;
    --i_step)
      I_STEP="$2"
      shift
      shift
      ;;
    --N_template)
      N_TEMPLATE="$2"
      shift
      shift
      ;;
    --next)
      NEXT="next"
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

N=$1
I_MAX=$2

# Retrieve N template
if [[ -z $N_TEMPLATE ]]; then
  N_TEMPLATE=$N
fi

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

MOD=""
# Build modality string
if [[ "$NEXT" != "" ]]; then
  MOD="next_"
fi

# Generation
for (( i=I_MIN; i<=I_MAX; i=i+I_STEP ))
do
  $EXEC $N $i $NEXT > "$OUT_DIR/crscounter_$MOD""N$N_TEMPLATE""_i$i.pltl"
done

