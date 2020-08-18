# BLACK [![Build Status](https://api.cirrus-ci.com/github/black-sat/black.svg)](https://cirrus-ci.com/github/black-sat/black)  ![MIT](https://img.shields.io/badge/license-MIT-brightgreen)

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

## Usage

See the next section for instructions on how to install the tool.

Once installed, run `black --help` for a brief usage help:
```
$ black --help

BLACK - Bounded Lᴛʟ sAtisfiability ChecKer


SYNOPSIS
   ./black [-k <bound>] [-B <name>] [<file>]
   ./black --sat-backends
   ./black -h

OPTIONS
   -k, --bound <bound>        maximum bound for BMC procedures
   -B, --sat-backend <name>   name of the selected SAT backend
   <file>                     input formula file name.
                              If missing, runs in interactive mode.
                              If '-', reads from standard input in batch mode.
   --sat-backends             print the list of available SAT backends
   -h, --help                 print this help message
```

The tool accepts a file name as a command line argument, and checks the 
satisfiability of the LTL formula contained in the file:

```
$ black benchmarks/formulas/acacia/demo-v3/demo-v3/demo-v3_1.pltl 
SAT
```

If launched without arguments, a formula is asked interactively:

```
$ black
Please enter formula: 
!(p | !p)
Parsed formula (nnf): !p & p

Solving...

The formula is UNSAT!
```

The `-k` (or `--bound`) command line option sets the maximum
number of iterations of the underlying algorithm (useful for very hard formulas).

## Installation

BLACK has been tested on Linux (Ubuntu ≥ 18.04 and Fedora ≥ 29) and macOS Mojave
and later, but it should work on any Linux or macOS system satisfying the
following requirements:
1. A C++17 compliant compiler, *e.g.* GCC ≥ 8.2 or clang ≥ 7
    (Xcode ≥ 10.2 available on macOS Mojave)
2. [CMake][CMake] 3.10 or upper (see below)
3. [zlib][zlib] 1.x
3. [Hopscotch-map][hopscotch] v.2.8

Optionally:

4. [MathSAT][MathSAT] 5.5 or later
5. [Z3][Z3] 4.4 or later
5. [CryptoMiniSAT][CMS] 5.x
6. [MiniSAT][MiniSAT] 2.2

Precompiled packages for major Linux distributions are being prepared. In the
meantime, the following setup instructions are given for Ubuntu 18.04 and newer.
On macOS Mojave and later, BLACK can be installed from Homebrew as shown below.

<details>
  <summary>macOS: installation from Homebrew</summary>
  
  Make sure to have installed the [Homebrew][Homebrew] package manager as
  explained on their website (very easy single-command installation). Then,
  install BLACK as follows:
  ```
  $ brew tap black-sat/black
  $ brew install black-sat
  ```
</details>

<details>
<summary>Linux: installation from sources</summary>

### Installing dependencies

<details>
<summary>Ubuntu ≥ 20.04 and other Debian-based distributions</summary>
On Ubuntu ≥ 20.04, install the required packages with `apt` as follows:

```
$ sudo apt install build-essential git gcc g++ cmake libtsl-hopscotch-map-dev libz-dev libfmt-dev
```
</details>
<details>
<summary>Fedora ≥ 29 and other RedHat-based distributions</summary>

On Fedora ≥ 29, install the required packages with `yum` as follows:
```
$ sudo yum install gcc gcc-c++ make cmake zlib-devel git
```
and then, install from source the `hopscotch-map` library, which is not available
as a precompiled `.rpm` package:
```
$ git clone https://github.com/Tessil/hopscotch-map.git
$ cd hopscotch-map
$ git checkout v2.3.0
$ cmake .
$ make install
```
</details>


### Compilation
1. Clone the repository and `cd` into the source directory:
   ```
   $ git clone https://github.com/black-sat/black.git
   $ cd black
   ```
2. <details><summary><strong>Optional</strong> Additional SAT backends</summary>
    The default backend SAT solver is Glucose, which is built into
   BLACK's sources and does not need to be installed separately. 
   
   The MiniSAT, CryptoMiniSAT, and Z3 backends will be automatically if their
   installation is found on the system.  
   
   To enable the MathSAT backend and install its dependencies, proceed as follows:
   <details>
   <summary>On Ubuntu and other Debian-based distributions</summary>

   ```
   $ sudo apt install libgmp-dev
   $ ./download-mathsat5.sh
   ```
   </details>
   <details>
   <summary>On Fedora and other Red Hat-based distributions</summary>

   ```
   $ sudo yum install gmp-devel gmp-c++
   $ ./download-mathsat5.sh
   ```
   </details>
   </details>

3. Create a `build` directory and `cd` into it:
   ```
   $ mkdir build && cd build
   ```
4. Run `cmake` inside the `build` directory.
   ```
   $ cmake ..
   ```
5. Build
   ```
   $ make
   ```
6. Run the tests if you want to make sure everything worked properly  
   ```
   $ make test
   ```
7. Install (usually requires administrator privileges)
   ```
   $ make install
   ```
</details>

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