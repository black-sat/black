.. highlight:: console

Installation on macOS
==============================

Easy installation with Homebrew
-------------------------------

The easiest way to install BLACK on macOS is through `Homebrew`_. It is a
popular package manager for open source applications on macOS. If you do not
have it installed already, installing it is very easy. Just follow the
instructions on their website.

Once Homebrew is installed, BLACK installation is as easy as typing this
command::

   $ brew install black-sat/black/black-sat

The Homebrew package works both on Intel and Apple Silicon systems.

.. note::
   There is a package called ``black`` in the Homebrew repository, 
   which is a popular code formatter for Python. If you run 
   ``brew install black``, you will install this other package, hence pay 
   attention to type the correct command as written above.

.. _Homebrew: https://brew.sh/index_it

If for any reason you want to compile BLACK from source, go ahead to the next
section.

Compilation from source
------------------------

Dependencies
~~~~~~~~~~~~

.. include:: dependencies.inc

Most of the dependencies are available through Homebrew::

   $ brew install cmake nlohmann-json fmt llvm

One package is installable through Homebrew but using our tap::

   $ brew install black-sat/black/hopscotch-map

Note that the `llvm` package is required because BLACK does not currently
compile with the stock Apple `clang` version, which does not support enough
C++20 features. 

Now at least one backend must be chosen and installed.

For Z3::

   $ brew install z3

For CryptoMiniSAT::

   $ brew install cryptominisat

Currently, there are no Homebrew packages for *cvc5* and *MathSAT*. To install
*cvc5*, stand-alone binary packages are available or it can be built from
source. Refer to their website for more information. To install *MathSAT*, we
provide a script to download the binary package and put it in a place where
BLACK's build system can find it.

From BLACK's source directory, run::

   $ ./download-mathsat.sh
   
This will unpack *MathSAT* into the ``external/`` subdirectory and will not
touch your system. The dependencies of *MathSAT* are available from Homebrew::

   $ brew install zlib gmp

Compilation
~~~~~~~~~~~~

First, clone the `GitHub`_ repository::

   $ git clone https://github.com/black-sat/black.git

.. _GitHub: https://github.com/black-sat/black

Now, we have to set the correct environment variables to use the `clang` 
compiler from the Homebrew `llvm` package::

   $ export CC=$(brew --prefix llvm)/bin/clang
   $ export CXX=$(brew --prefix llvm)/bin/clang++
   $ export LDFLAGS="-L$(brew --prefix llvm)/lib -Wl,-rpath,$(brew --prefix llvm)/lib"
   $ export CXXFLAGS="-I$(brew --prefix llvm)/include"

.. include:: compilation.inc