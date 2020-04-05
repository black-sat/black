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
   black [-k <bound>] [<file>]
   black -h

OPTIONS
   -k, --bound <bound>   maximum bound for BMC procedures
   <file>                input formula file name.
                         If missing, runs in interactive mode.
                         If '-', reads from standard input in batch mode.
   -h, --help            print this help message
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

BLACK has been tested on Linux (Ubuntu 18.04 and Fedora ≥ 29) and
macOS Mojave, but it should work on any Linux or macOS system satisfying the
following requirements:
1. A C++17 compliant compiler, *e.g.* GCC ≥ 8.2 and clang ≥ 7
    (Xcode ≥ 10.2 available on macOS Mojave)
2. [CMake][CMake] 3.10 or upper (see below)
3. [MathSAT][MathSAT] 5.5 or later (see below)
   * GNU GMP library (see below)
   * ZLib (see below)

Precompiled packages for macOS and major Linux distributions are being prepared.
In the meantime, the following setup instructions are given for Ubuntu 18.04 and
newer, and for macOS Mojave, and may need to be adapted for different versions.

#### WARNING
Please beware of this compatibility caveat before reporting a build failure.

GCC 7, shipped by default on Ubuntu 18.04, and the Apple Clang
shipped with Xcode 10.1 (up to Mac OS X High Sierra) claim C++17
compatibility but lack proper support for some required standard library
component.

Hence, please use GCC ≥ 8.2, upstream Clang ≥ 7, or Xcode ≥ 10.2
(macOS Mojave). For Ubuntu 18.04 or later and macOS Mojave, follow the
following instructions, or adapt them to your system/distribution.

### Installing dependencies

On Ubuntu 18.04, install the required packages with `apt` as follows:

```
$ sudo apt install build-essential gcc-8 g++-8 cmake libgmp-dev libz-dev git wget
```

On Fedora ≥ 29:

```
$ sudo yum install gcc gcc-c++ make cmake gmp-devel zlib-devel git wget
```

On macOS Mojave, install the [Homebrew] package manager as explained on their
website (very easy single-command installation), then install the required
dependencies as follows:

```
$ xcode-select --install # make sure you have Apple Command Line Developer Tools
$ brew install cmake gmp wget
```

If the first command shows a dialog window, make sure to click on *Install*,
not on *Get Xcode*

### Compilation
1. Clone the repository. Clone it recursively, to get all the submodules, and
   `cd` into the source directory:
   ```
   $ git clone https://github.com/black-sat/black.git
   $ cd black
   ```
2. Download and unpack the MathSAT 5 distribution:
   ```
   $ ./download-mathsat5.sh
   ```
1. Create a `build` directory and `cd` into it:
   ```
   $ mkdir build && cd build
   ```
2. Run `cmake` inside the `build` directory.
   * on Ubuntu (and Linux systems in general), make sure to select GCC 8:  
      ```
      $ CC=gcc-8 CXX=g++-8 cmake ..
      ```
   * On macOS Mojave, the default choice is fine:  
      ```
      $ cmake ...
      ```
3. Build
   ```
   $ make
   ```
4. Run the tests if you want to make sure everything worked properly  
   ```
   $ make test
   ```
5. Install (usually requires administrator privileges)
   ```
   $ make install
   ```


[Reynolds]: https://arxiv.org/abs/1609.04102
[CMake]: https://cmake.org
[MathSAT]: http://mathsat.fbk.eu
[Homebrew]: https://brew.sh
[Geatti]: https://users.dimi.uniud.it/~luca.geatti
[Gigante]: https://users.dimi.uniud.it/~nicola.gigante
[Montanari]: https://users.dimi.uniud.it/~angelo.montanari
[Tableaux]: https://tableaux2019.org/
[Paper]: https://users.dimi.uniud.it/~nicola.gigante/papers/GeattiGM19.pdf
[DBLP]: https://dblp.org/rec/conf/tableaux/GeattiGM19.html