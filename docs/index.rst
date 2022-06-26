.. black-sat documentation master file, created by
   sphinx-quickstart on Fri Jun 24 16:09:31 2022.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

=====================================
BLACK
=====================================
**Bounded Lᴛʟ sAtisfiability ChecKer**

Welcome to **BLACK**'s website.

BLACK is a tool for testing the satisfiability of formulas in Linear
Temporal Logic (LTL) and related logics. 

LTL is a very popular specification language in the fields of *formal
verification* and *artificial intelligence*. In these fields, it is important to
be able to understand whether a given specification is satisfiable,
unsatisfiable, or vacuously true. BLACK helps with this task.

BLACK consists of two components:

1. ``libblack``, a C++ library with a well-defined API that can be linked by  
any client application; and

2. ``black``, a command-line tool that interfaces the library with the user.

This website documents the usage of both.

BLACK is open-source and released under the MIT license. For its source code,
look at its `GitHub repository <https://github.com/black-sat/black>`_. 

.. toctree::
   :maxdepth: 2
   :caption: Contents

   installation
   logics
   cli
   syntax
   api
   publications


:ref:`genindex`
