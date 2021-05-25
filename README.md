# BLACK [![Build Status](https://api.cirrus-ci.com/github/black-sat/black.svg)](https://cirrus-ci.com/github/black-sat/black) ![appveyor](https://ci.appveyor.com/api/projects/status/github/black-sat/black?branch=master&svg=true) ![MIT](https://img.shields.io/badge/license-MIT-brightgreen) [![Latest release](https://badgen.net/github/release/black-sat/black)](https://github.com/black-sat/black/releases/tag/v0.6.1) [![codecov](https://codecov.io/gh/black-sat/black/branch/master/graph/badge.svg?token=ZETQF5NZ6X)](https://codecov.io/gh/black-sat/black)

BLACK (short for Bounded Lᴛʟ sAtisfiability ChecKer) is a tool for testing the
satisfiability of LTL and LTLf formulas based on the SAT encoding of the tableau
method described [here][Reynolds]. An in depth description of the encoding and
the whole algorithm has been published in the proceedings of the TABLEAUX 2019
conference.

[Luca Geatti][Geatti], [Nicola Gigante][Gigante], [Angelo Montanari][Montanari]  
A SAT-based encoding of the one-pass and tree-shaped tableau system for LTL. 
In: *Proceedings of the 28th International Conference on Automated Reasoning with 
Analytic Tableaux and Related Methods*, [TABLEAUX 2019][Tableaux], pages 3‑20  
DOI: 10.1007/978-3-030-29026-9_1  
[Full Text][Paper], [DBLP][DBLP]

BLACK is:
* **Fast**: based on a state-of-the-art SAT-based encoding 
* **Lightweight**: low memory consuption even for large formulas
* **Flexible**: supports LTL and LTL+Past both on infinite and finite models
* **Robust**: rock-solid stability with 100% test coverage
* **Multiplatform**: works on Linux, macOS and Windows
* **Easy to use**: easy to install binary packages provided for all platforms
* **Embeddable**: use BLACK's library API to integrate BLACK's solver into your code

See the [Documentation][Home] on how to use BLACK. In particular:

1. [Downloads and Installation][Installation]
2. [Usage][Usage]
3. (coming soon...) BLACK Library API

Quick usage help:
```
$ black --help

BLACK - Bounded Lᴛʟ sAtisfiability ChecKer
        version 0.6.1


SYNOPSIS
   ./black solve [-k <bound>] [-B <backend>] [--remove-past] [--finite] [-m]
           [-o <fmt>] [-f <formula>] [<file>]

   ./black check -t <trace> [-e <result>] [-i <state>] [--finite] [--verbose]
           [-f <formula>] [<file>]

   ./black dimacs [-B <backend>] <file>
   ./black --sat-backends
   ./black -v
   ./black -h

OPTIONS
   solving mode: 
       -k, --bound <bound>         maximum bound for BMC procedures
       -B, --sat-backend <backend> select the SAT backend to use
       --remove-past               translate LTL+Past formulas into LTL before
                                   checking satisfiability

       --finite                    treat formulas as LTLf and look for finite
                                   models

       -m, --model                 print the model of the formula, if any
       -o, --output-format <fmt>   Output format.
                                   Accepted formats: readable, json
                                   Default: readable
       -f, --formula <formula>     LTL formula to solve
       <file>                      input formula file name.
                                   If '-', reads from standard input.

   trace checking mode: 
       -t, --trace <trace>         trace file to check against the formula.
                                   If '-', reads from standard input.
       -e, --expected <result>     expected result (useful in testing)
       -i, --initial-state <state> index of the initial state over which to
                                   evaluate the formula. Default: 0

       --finite                    treat formulas as LTLf and expect a finite
                                   model

       --verbose                   output a verbose log
       -f, --formula <formula>     formula against which to check the trace
       <file>                      formula file against which to check the
                                   trace

   DIMACS mode: 
       -B, --sat-backend <backend> select the SAT backend to use
       <file>                      DIMACS file to solve

   --sat-backends                  print the list of available SAT backends
   -v, --version                   show version and license information
   -h, --help                      print this help message
```


[Reynolds]: https://arxiv.org/abs/1609.04102
[CMake]: https://cmake.org
[zlib]: https://zlib.net/
[hopscotch]: https://github.com/Tessil/hopscotch-map
[CMS]: https://github.com/msoos/cryptominisat
[MiniSAT]: http://minisat.se/
[Z3]: https://github.com/Z3Prover/z3
[MathSAT]: http://mathsat.fbk.eu
[Homebrew]: https://brew.sh
[Geatti]: https://users.dimi.uniud.it/~luca.geatti
[Gigante]: https://users.dimi.uniud.it/~nicola.gigante
[Montanari]: https://users.dimi.uniud.it/~angelo.montanari
[Tableaux]: https://tableaux2019.org/
[Paper]: https://users.dimi.uniud.it/~nicola.gigante/papers/GeattiGM19.pdf
[DBLP]: https://dblp.org/rec/conf/tableaux/GeattiGM19.html
[Home]: https://github.com/black-sat/black/wiki/Home 
[Installation]: https://github.com/black-sat/black/wiki/Installation 
[Usage]: https://github.com/black-sat/black/wiki/Usage 
