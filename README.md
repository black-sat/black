# BLACK [![Build Status](https://api.cirrus-ci.com/github/black-sat/black.svg)](https://cirrus-ci.com/github/black-sat/black)  ![MIT](https://img.shields.io/badge/license-MIT-brightgreen) [![Latest release](https://badgen.net/github/release/black-sat/black)](https://github.com/black-sat/black/releases/tag/v0.2.0)

BLACK (short for Bounded Ltl sAtisfiability ChecKer) is a tool for testing the
satisfiability of LTL formulas based on the SAT encoding of the tableau method
described [here][Reynolds]. An in depth description of the encoding and the
whole algorithm has been published in the proceedings of the TABLEAUX 2019 
conference.

[Luca Geatti][Geatti], [Nicola Gigante][Gigante], [Angelo Montanari][Montanari]  
A SAT-based encoding of the one-pass and tree-shaped tableau system for LTL. 
In: *Proceedings of the 28th International Conference on Automated Reasoning with 
Analytic Tableaux and Related Methods*, [TABLEAUX 2019][Tableaux], pages 3‑20  
DOI: 10.1007/978-3-030-29026-9_1  
[Full Text][Paper], [DBLP][DBLP]

See the project's wiki for the [Documentation][Home]. In particular:

1. [Installation][Installation]
2. [Usage][Usage]
3. (coming soon...) BLACK Library API

Quick installation instructions:

| Ubuntu ≥ 20.04             | Fedora ≥ 32, CentOS/RHEL&nbsp;≥&nbsp;8 | macOS ≥ 10.14 with [Homebrew][Homebrew] |
|----------------------------|------------------------------|-----------------------------|
| [![Download](https://badgen.net/badge/Download%20v0.2.0/.deb/green)][pkg.deb] | [![Download](https://badgen.net/badge/Download%20v0.2.0/.rpm/green)][pkg.rpm]| |
| How to install:<br/>`$ sudo apt install ⟨file⟩` | How to install:<br/>`$ sudo dnf install ⟨file⟩` |How to install:<br/>`$ brew install black-sat/black/black-sat`|

Quick usage help:
```
$ black --help

BLACK - Bounded Lᴛʟ sAtisfiability ChecKer
        version 0.1.1


SYNOPSIS
   black [-k <bound>] [-B <name>] [<file>]
   black --sat-backends
   black -v
   black -h

OPTIONS
   -k, --bound <bound>        maximum bound for BMC procedures
   -B, --sat-backend <name>   select the SAT backend to use
   <file>                     input formula file name.
                              If missing, runs in interactive mode.
                              If '-', reads from standard input in batch mode.
   --sat-backends             print the list of available SAT backends
   -v, --version              show version information
   -h, --help                 print this help message
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
[pkg.deb]: https://github.com/black-sat/black/releases/download/v0.2.0/black-sat-0.2.0-1.x86_64.deb
[pkg.rpm]: https://github.com/black-sat/black/releases/download/v0.2.0/black-sat-0.2.0-1.x86_64.rpm
