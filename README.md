# BLACK [![Build Status](https://api.cirrus-ci.com/github/black-sat/black.svg)](https://cirrus-ci.com/github/black-sat/black) ![appveyor](https://ci.appveyor.com/api/projects/status/github/black-sat/black?branch=master&svg=true) ![MIT](https://img.shields.io/badge/license-MIT-brightgreen) [![Latest release](https://badgen.net/github/release/black-sat/black)](https://github.com/black-sat/black/releases/tag/v0.7.1) [![codecov](https://codecov.io/gh/black-sat/black/branch/master/graph/badge.svg?token=ZETQF5NZ6X)](https://codecov.io/gh/black-sat/black)

BLACK (short for Bounded Lᴛʟ sAtisfiability ChecKer) is a tool for testing the
satisfiability of LTL and LTLf formulas based on the SAT encoding of the tableau
method described [here][Reynolds]. An in depth description of the encoding and
the whole algorithm has been published in the proceedings of the TABLEAUX 2019
conference (see the Publications section below).

BLACK is:
* **Fast**: based on a state-of-the-art SAT-based encoding 
* **Lightweight**: low memory consuption even for large formulas
* **Flexible**: supports LTL and LTL+Past both on infinite and finite models
* **Robust**: rock-solid stability with 100% test coverage
* **Multiplatform**: works on Linux, macOS, Windows and FreeBSD
* **Easy to use**: easy to install binary packages provided for all platforms
* **Embeddable**: use BLACK's library API to integrate BLACK's solver into your code

See the [Documentation][Home] on how to use BLACK. In particular:

1. [Downloads and Installation][Installation]
2. [Usage][Usage]
3. [Input Syntax][Syntax]
4. [Publications][Publications]
5. (coming soon...) BLACK C++ Library API

[Reynolds]: https://arxiv.org/abs/1609.04102
[CMake]: https://cmake.org
[zlib]: https://zlib.net/
[hopscotch]: https://github.com/Tessil/hopscotch-map
[CMS]: https://github.com/msoos/cryptominisat
[MiniSAT]: http://minisat.se/
[Z3]: https://github.com/Z3Prover/z3
[MathSAT]: http://mathsat.fbk.eu
[Homebrew]: https://brew.sh
[Home]: https://github.com/black-sat/black/wiki/Home 
[Installation]: https://github.com/black-sat/black/wiki/Installation 
[Publications]: https://github.com/black-sat/black/wiki/Publications 
[Syntax]: https://github.com/black-sat/black/wiki/Syntax 
[Usage]: https://github.com/black-sat/black/wiki/Usage 
