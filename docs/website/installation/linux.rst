.. highlight:: console

Installation on Linux
=====================

Binary packages
----------------

We provide binary packages for **Ubuntu 24.04**, (the latest LTS) and **Fedora 40**.

.. list-table::
   :header-rows: 1

   * - Ubuntu 24.04
     - Fedora 40
   * - |Ubuntu24.04Badge|_
     - |Fedora40Badge|_
   * - ``sudo apt install ⟨file⟩``
     - ``sudo dnf install ⟨file⟩``

These packages should work on other Debian-based and RedHat-based distributions
as well, and include the `Z3`_ backend. If they do not work for you, or if you
want to use other backends, BLACK can be compiled from source. If you think that
the packages should work but do not, please open a GitHub `issue
<https://github.com/black-sat/black/issues>`_.


.. |Ubuntu24.04Badge| image:: https://badgen.net/badge/Download%20v0.10.8/.deb/green
.. _Ubuntu24.04Badge: https://github.com/black-sat/black/releases/download/v0.10.8/black-sat-0.10.8.ubuntu24.04.x86_64.deb
.. |Fedora40Badge| image:: https://badgen.net/badge/Download%20v0.10.8/.rpm/green
.. _Fedora40Badge: https://github.com/black-sat/black/releases/download/v0.10.8/black-sat-0.10.8.fedora40.x86_64.rpm

Compilation from source
------------------------

.. _cloning: 

To retrieve BLACK's source code, clone the `GitHub`_ repository::

   $ git clone https://github.com/black-sat/black.git

.. _GitHub: https://github.com/black-sat/black

Dependencies
~~~~~~~~~~~~

BLACK can be compiled by any C++20 compliant compiler. On Linux, we require
Clang 13 or above, and g++ 10 or above.

.. include:: dependencies.inc

Debian-based distributions
++++++++++++++++++++++++++++

Most of the required dependencies can be installed from the package repository::
   
   $ sudo apt install build-essential cmake libtsl-hopscotch-map-dev libfmt-dev

Unfortunately, ``nlohmann-json`` is not packaged, so it needs to be installed
from source. That is however very easy.

.. include:: nlohmann.inc

Now at least one backend must be chosen and installed.

For Z3::

   $ sudo apt install libz3-dev

For CryptoMiniSAT::

   $ sudo apt install libcryptominisat5-dev

For MiniSAT::

   $ sudo apt install minisat

Currently, there are no packages for cvc5 and MathSAT. To install cvc5, binary
packages are available or it can be built from source. Refer to their website
for more information. For MathSAT, we provide a script to download the binary
package and put it in a place where BLACK's build system can find it.

From BLACK's source directory, run::

   $ ./download-mathsat.sh
   
This will unpack MathSAT into the ``external/`` subdirectory and will not touch
your system. The dependencies of MathSAT are available from the package
repository::

   $ sudo apt install libz-dev libgmp-dev  

RedHat-based distributions
++++++++++++++++++++++++++++

Most of the required dependencies can be installed from the package repository::
   
   $ sudo dnf install make cmake gcc gcc-c++ fmt-devel

Unfortunately, ``hopscotch-map`` and  ``nlohmann-json`` are not packaged, so
they need to be installed from source. That is however very easy.

To install ``hopscotch-map``::

   $ git clone https://github.com/Tessil/hopscotch-map.git 
   $ cd hopscotch-map
   $ git switch v2.3.0
   $ mkdir build && cd build
   $ cmake ..
   $ make
   $ sudo make install

.. include:: nlohmann.inc

Now at least one backend must be chosen and installed.

For Z3::

   $ sudo dnf install z3-devel

For CryptoMiniSAT::

   $ sudo dnf install cryptominisat-devel

For MiniSAT::

   $ sudo dnf install minisat2-devel

Currently, there are no ``rpm`` packages for cvc5 and MathSAT. To install cvc5,
stand-alone binary packages are available or it can be built from source. Refer
to their website for more information. To install MathSAT, we provide a script
to download the binary package and put it in a place where BLACK's build system
can find it.

From BLACK's source directory, run::

   $ ./download-mathsat.sh
   
This will unpack MathSAT into the ``external/`` subdirectory and will not touch
your system. The dependencies of MathSAT are available from the package
repository::

   $ sudo dnf install zlib-devel gmp-devel gmp-c++

Compilation
~~~~~~~~~~~~

Once installed all the dependencies, you can compile BLACK. First :ref:`clone
<cloning>` the repository (say into the ``black`` directory).

.. include:: compilation.inc