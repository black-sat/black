Installation on Windows
========================

Binary package
---------------

On Windows, we provide a binary package in the form of a `zip` file containing
the `black` command-line tool, the `libblack.dll` library, and all the needed
header files and import libraries. No installation is needed, the `zip` archive
is self-contained.

.. list-table::

   * - |WindowsBadge|_

.. |WindowsBadge| image:: https://badgen.net/badge/Download%20v0.10.1/.zip/green
.. _WindowsBadge: https://github.com/black-sat/black/releases/download/v0.10.1/black-0.10.1-win-x64.zip


The binary package includes the Z3 backend. To use other backends, BLACK needs
to be built from source. Instructions from :doc:`Linux <linux>` can be easily
adapted, excepting that dependencies must be installed by hand.