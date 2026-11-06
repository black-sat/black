#!/usr/bin/env bash

set -e # script exits at the first error

should_fail() {
  if "$@"; then
    return 1
  fi
}

# 
# The following is a trick to have bash print the line of the first command that
# fails.
# See https://unix.stackexchange.com/a/298927/196591
#

cur_command=
first_err_command=
first_err_lineno=
trap 'cur_command=$BASH_COMMAND;
      if [[ -z "$first_err_command" ]]; then
          first_err_command=$cur_command;
          first_err_lineno=$LINENO;
      fi' ERR
trap 'if [[ ! -z "$first_err_command" ]]; then
          echo "ERROR: Aborting at line: $first_err_lineno on command: $first_err_command";
      fi' EXIT




# should_fail ./black check -t ../tests/test-trace.json --safety
# should_fail ./black check -t - -f 'p' file.pltl --safety
# should_fail ./black check -t - - --safety
# should_fail ./black check -t --safety
# should_fail ./black check -t ../tests/test-trace.json -f !p --safety

should_fail ./black check -t ../tests/test-trace-mt.json -f 'G(x > -3)' --safety

should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f 'G(x+1= next(x))' --safety --cosafety
should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f 'F(x = 0)' --safety
should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f '!G(x = 0)' --safety
should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f 'G(x = 0)' --cosafety
should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f '!F(x = 0)' --cosafety
should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f 'F(G( x > -3 S y = 3))' --cosafety

should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f 'G(x+1= next(x))' --safety
should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f 'F(X (x > 0))' --cosafety
should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f 'F(x > 6.5)' --cosafety

should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f 'F(x)' --cosafety

cat <<END | should_fail ./black check -d Real -t - -f 'F(z > 6.5)' --cosafety
{
  "model": {
    "size": 1,
    "loop": 0,
    "states": [
      {
        "x": -2,
        "z" : "true"
      }
    ]
  }
}
END

cat <<END | should_fail ./black check -d Real -t - -f 'F(x = -2)' --cosafety
{
  "model": {
    "size": 1,
    "loop": 1,
    "states": [
      {
        "x": -2
      }
    ]
  }
}
END

should_fail ./black check -d Int -t ../tests/test-trace-mt.json -f 'G(exists x : Int . x = 0)' --safety

cat <<END | ./black check -d Int -t - -f 'F( x > -3 S y = 3)' --cosafety
{
  "model": {
    "size": 3,
    "states": [
      {
        "y": 2,
        "x": -5
      },
      {
        "y": 3,
        "x": 2
      },
      {
        "x": 1
      }
    ]
  }
}
END

cat <<END | ./black check -d Int -t - -f 'G( p && (x > -3 S y = 3))' --safety
{
  "model": {
    "size": 3,
    "states": [
      {
        "p": "true",
        "y": 3,
        "x": -2
      },
      {
        "p": "true",
        "y": 4,
        "x": 2
      },
      {
        "p": "true",
        "x": 1
      }
    ]
  }
}
END

./black check -d Int -t ../tests/test-trace-mt.json -f 'G(x+1= wprev(x))' --safety