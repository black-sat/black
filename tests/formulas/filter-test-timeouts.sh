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


die() {
  echo \
This script must be executed from the root directory of leviathan\'s source \
tree. 1>&2
  exit 1
}

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

# Check that we run from the topmost source dir
[ -d .git ] || die

while IFS=", " read -r -a LINE; do
  # Parse the line
  filename=${LINE[0]}
  result=${LINE[8]}

  # Discard CSV header
  [ $filename != "benchmark" ] || continue

  # Discard entries with unknown result
  [ $result != "UNK" ] || continue

  # Launch checker
  if mytimeout ./bin/checker --verbosity 0 "$filename"; then
    echo "$filename;$result" | sed 's/UNS/UNSAT/'
  fi
done 
