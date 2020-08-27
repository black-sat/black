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

  $command --foreground -s KILL 0.01s "$@"

  # timeout returns 124 or 137 if it had to kill the process
  # Yes I love Unix...
  [ $? -ne 124 -a $? -ne 137 ] && return 0 || return 1
}

# Run from the topmost source dir
cd $(git rev-parse --show-toplevel)

summaryfile=tests/formulas/tests.index

cat $summaryfile | while IFS=";" read -r -a LINE; do
  # Parse the line
  filename=$(echo ${LINE[0]})
  result=$(echo ${LINE[1]})

  if [ "$result" = "UNSAT" ]; then
    echo "$filename;$result"
    continue
  fi

  # Launch checker
  if mytimeout ./build/black "./benchmarks/formulas/$filename" > /dev/null; then
    echo "$filename;$result"
  fi
done
