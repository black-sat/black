#!/bin/bash

#
# This script processes a CSV file obtained by StarExec 'job information'
# command and extract all and only the benchmarks that complete in at most 3
# seconds.
#
# The output file format is suitable to be parsed by CMakeLists.txt when
# configuring CTest, i.e. each line is a test file followed by "SAT" or
# "UNSAT", separated by a semicolon
#

# Wrapper for the timeout command
mytimeout()
{
  # The command has a different name when installed by Homebrew
  command=timeout
  [ "$(uname)" = "Darwin" ] && command=gtimeout

  $command --foreground -s KILL 3s $@

  # timeout returns 124 or 137 if it had to kill the process
  # Yes I love Unix...
  [ $? -ne 124 -a $? -ne 137 ] && return 0 || return 1
}

# Run from the topmost source dir
cd $(git rev-parse --show-toplevel)

summaryfile=tests/formulas/starexec/NuSMV-30min.csv

cat $summaryfile | grep BDD | while IFS=", " read -r -a LINE; do
  # Parse the line
  filename=$(echo ${LINE[0]} | sed 's/ATVA11pltl-orig\///g')
  result=$(echo ${LINE[8]} | sed 's/UNS/UNSAT/')

  # Discard CSV header
  [ "$filename" != "benchmark" ] || continue

  # Discard entries with unknown result
  [ "$result" != "UNK" -a "$result" != "--" ] || continue

  # Launch checker
  if mytimeout ./build/black -f "./tests/formulas/$filename" > /dev/null; then
    echo "$filename;$result"
  fi
done
