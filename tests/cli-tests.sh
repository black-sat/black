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

./black --help
./black --version
./black --sat-backends

./black solve -m -f 'G F (p && !q)' | grep -w SAT
./black solve -m --finite -f 'G F (p && !q)' | grep -w SAT
./black solve -f 'p && !p' | grep -w UNSAT
./black solve -k 1 -f 'G (Z False || Y !p2)' | grep UNKNOWN
./black solve --remove-past -f 'G (Z False || Y !p2)' | grep SAT
./black solve -c -f 'G((p U (q & w)) & c) & F((r U s) & !c)' | grep UNSAT
./black solve -c -f 'G((p U (q & w)) & c) & F((r U True) & False)' --debug uc-replacements | \
  grep 'MUC: {0} & F({1} & False)'
./black solve -o json -f 'p & !p' | ./black check -t - -f 'p & !p'
./black solve -o json -f 'p & q' | ./black check -t - -f 'p & q'

echo G F p | ./black solve -
should_fail ./black solve non-existent.pltl
should_fail ./black solve -f 'F' # syntax error
should_fail ./black solve -o json -f 'F' # syntax error in JSON format
should_fail ./black solve 
should_fail ./black solve -f 'p' file.pltl
should_fail ./black 
should_fail ./black solve -o
should_fail ./black solve -s -d integers -c -f 'x = 0 & x != 0'

should_fail ./black solve --remove-past -s -d integers -f 'F H(x = 0)'
should_fail ./black solve -m -d integers -f 'x = 0'

should_fail ./black solve -B cmsat -s -d integers -f 'x = 0'
should_fail ./black solve -B mathsat -s -d integers -f 'exists x . x = 0'
should_fail ./black solve -s -f 'x = 0'

./black solve -d integers -f 'next(x) = 0' 2>&1 | grep -- '--semi-decision'
./black solve -d integers -f 'wnext(x) = 0' 2>&1 | grep -- '--semi-decision'
./black solve -d integers -f 'prev(x) = 0' 2>&1 | grep -- '--semi-decision'
./black solve -d integers -f 'wprev(x) = 0' 2>&1 | grep -- '--semi-decision'
./black solve -d integers -f 'r(next(x))' 2>&1 | grep -- '--semi-decision'
./black solve -d integers -f 'r(wnext(x))' 2>&1 | grep -- '--semi-decision'
./black solve -d integers -f 'r(prev(x))' 2>&1 | grep -- '--semi-decision'
./black solve -d integers -f 'r(wprev(x))' 2>&1 | grep -- '--semi-decision'

./black solve -f 'p & q' --debug print
./black solve -f 'X p & X X q & F(q)' --debug trace
./black solve -f 'X p & X X q & F(q)' --debug trace-full

should_fail ./black check -t ../tests/test-trace.json
should_fail ./black check -t - -f 'p' file.pltl
should_fail ./black check -t - -
should_fail ./black check -t 
should_fail ./black check -t ../tests/test-trace.json -f !p

./black check -t ../tests/test-trace.json <(echo p)
./black check -t ../tests/test-trace.json --verbose -i 0 -e SAT -f p
cat ../tests/test-trace.json | ./black check -t - -f p
echo p | ./black check -t ../tests/test-trace.json -

./black solve -m -o json -f 'G F p' | \
  should_fail ./black check -t - --finite -f 'G F p'

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

cat <<END | should_fail ./black check -t - -f 'p'
{
  "model": {
    "size": 0,
    "loop": 0,
    "states": []
  }
}
END

cat <<END | should_fail ./black check -t - -f 'p'
{
  "model": {
    "size": 1,
    "loop": 1,
    "states": [
      {
        "p": "yes"
      }
    ]
  }
}
END

cat <<END | should_fail ./black check -t - -f 'p'
{
  "model": {
    "size": 1,
    "loop": 1,
    "states": [
      {
        "p": "true"
      },
      {
        "p": "false"
      }
    ]
  }
}
END

cat <<END | should_fail ./black check -t - -f 'p'
{
  "model": {
    "size": 1,
    "loop": 2,
    "states": [
      {
        "p": "true"
      }
    ]
  }
}
END

cat <<END | ./black check -t - -f 'q'
{
  "model": {
    "size": 1,
    "loop": 1,
    "states": [
      {
        "p": "true"
      }
    ]
  }
}
END

cat <<END | should_fail ./black check -t - -f 'p'
{
    "result": "UNSAT",
    "k": 1,
    "muc": "G("
}
END

cat <<END | should_fail ./black check -t - -f 'p & !p'
{
    "result": "UNSAT",
    "k": 1,
    "muc": "p & {0}"
}
END