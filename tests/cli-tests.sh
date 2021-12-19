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

./black solve -m -f 'G F (p && !q)' | grep -w SAT
./black solve -m --finite -f 'G F (p && !q)' | grep -w SAT
./black solve -f 'p && !p' | grep -w UNSAT
./black solve -k 1 -f 'G (Z False || Y !p2)' | grep UNKNOWN
./black solve --remove-past -f 'G (Z False || Y !p2)' | grep SAT
echo G F p | ./black solve -
should_fail ./black solve non-existent.pltl
should_fail ./black solve -f 'F' # syntax error
should_fail ./black solve -o json -f 'F' # syntax error in JSON format
should_fail ./black solve 
should_fail ./black solve -f 'p' file.pltl
should_fail ./black 
should_fail ./black solve -o

should_fail ./black solve --remove-past -s -d integers -f 'F H(x = 0)'
should_fail ./black solve -m -d integers -f 'x = 0'

should_fail ./black solve -s -d reals -f 'x = 0.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001'

should_fail ./black solve -B cmsat -s -d integers -f 'x = 0'
should_fail ./black solve -B mathsat -s -d integers -f 'exists x . x = 0'
should_fail ./black solve -s -f 'x = 0'

./black solve -d integers -f 'next(x) = 0' 2>&1 | grep -- '--semi-decision'

./black solve -f 'p & q' --debug print
./black solve -f 'X p & X X q & F(q)' --debug trace
./black solve -f 'X p & X X q & F(q)' --debug trace-full

should_fail ./black check -t ../tests/test-trace.json
should_fail ./black check -t - -f 'p' file.pltl
should_fail ./black check -t - -
should_fail ./black check -t 
should_fail ./black check -t ../tests/test-trace.json -f !p
should_fail ./black check --trace - -f 'x = 0'

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

./black dimacs ../tests/test-dimacs-sat.cnf | grep -w SATISFIABLE 
./black dimacs ../tests/test-dimacs-unsat.cnf | grep -w UNSATISFIABLE 

if ./black --sat-backends | grep mathsat; then
  ./black dimacs -B mathsat ../tests/test-dimacs-sat.cnf | grep -w SATISFIABLE 
fi

cat <<END | should_fail ./black dimacs -
p cnf
END

cat <<END | should_fail ./black dimacs -
c missing header
1 2 3 0
END

cat <<END | should_fail ./black dimacs -
p cnf 1 3
1 2 3 
END

cat <<END | should_fail ./black dimacs -
p cnf 1 3
1 2 a 0
END