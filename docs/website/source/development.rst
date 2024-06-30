Development environment
========================

Build dependencies
-------------------

BLACK's development depend on the following components:

1. a C++23 compliant compiler and standard library (Clang 18+, GCC 14+)
2. ``cvc5`` 1.1.2 or later (https://cvc5.github.io/)
3. the ``immer`` C++ library of persistent data structures (https://sinusoid.es/immer/)
4. the Catch2 testing library (https://github.com/catchorg/Catch2)
5. to build Python bindings, Python 3.10 or later, and the ``pybind11`` C++ library (https://github.com/pybind/pybind11)

All dependencies should be easily installable using Homebrew on macOS or the
distribution's package manager on Linux systems. The only exception is ``cvc5``
which has to be installed from the binary distribution available on their
website or recompiled from source.

.. note:: 
    The version of Clang bundled with **macOS** is not enough up-to-date. 
    Please install the latest version with Homebrew (``brew install llvm``) and 
    follow the instructions in the note below to use this version for building 
    BLACK.

Building instructions
-----------------------

Building is done using CMake. It is advisable to configure the build to install in a local directory (``~/.local`` in the following example).

From BLACK's source directory::

    $ mkdir build && cd build
    $ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/.local -DBLACK_ENABLE_PYTHON_BINDINGS=YES ..
    $ make 

Refer to ``docs/README.md`` for how to build the documentation.

.. note::
    On **macOS**, to use the Clang version installed with Homebrew, a few 
    additional CMake parameters are needed. The ``config-brew-llvm.sh`` script 
    does this. It accepts as arguments all the additional arguments to pass to 
    CMake, so one must replace the second line above with the following::

        $ ../config-brew-llvm.sh -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=~/.local -DBLACK_ENABLE_PYTHON_BINDINGS=YES ..