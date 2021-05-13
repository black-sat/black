#!/usr/bin/env bash

set -e # script exits at the first error

should_fail() {
  if "$@"; then
    return 1
  fi
}

./black --help
./black --version
./black --sat-backends

./black solve -m -f 'G F p' | grep -v UNSAT
echo G F p | ./black solve -

should_fail ./black solve non-existent.pltl
should_fail ./black solve -f 'F' # syntax error
should_fail ./black solve -o json -f 'F'
should_fail ./black solve 
should_fail ./black solve -f 'p' file.pltl
should_fail ./black 
should_fail ./black solve -o

should_fail ./black check -t ../tests/test-trace.json
should_fail ./black check -t -f 'p' file.pltl
should_fail ./black check -t - -
should_fail ./black check -t 
should_fail ./black check -t ../tests/test-trace.json -f !p

./black check -t ../tests/test-trace.json --verbose -i 0 -e SAT -f p
cat ../tests/test-trace.json | ./black check -t - -f p

cat <<END | should_fail ./black check -t - -e SAT -f 'p'
{
  "result": "UNSAT"
}
END

cat <<END | should_fail ./black check -t - -f 'p' # syntax error
{
  "result" = "UNSAT"
}
END