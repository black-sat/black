Then, compilation and installation are easy::

   $ cd black
   $ mkdir build 
   $ cd build
   $ cmake ..
   $ make 
   $ sudo make install

To configure the build to your needs, ``cmake`` accepts additional options that
can be passed as additional parameters with the syntax ``-DOPTION=value``.

1. ``ENABLE_MATHSAT=YES/NO``: whether to enable the MathSAT backend (default 
YES, if found)

2. ``ENABLE_CMSAT=YES/NO``: whether to enable the CryptoMiniSAT backend 
(default YES, if found)

3. ``ENABLE_Z3=YES/NO``: whether to enable the Z3 backend (default YES, if 
found)

4. ``ENABLE_CVC5=YES/NO``: whether to enable the cvc5 backend (default YES, if 
found)

5. ``ENABLE_MINISAT=YES/NO``: whether to enable the MiniSAT backend (default 
   YES, if found)

6. | ``BLACK_DEFAULT_BACKEND=<backend>``: default backend (default: ``z3``, if 
     found).
   | Accepted values: ``mathsat``, ``cmsat``, ``z3``, ``cvc5``, ``minisat``

7. ``ENABLE_FORMULAS_TESTS=YES/NO``: whether to enable the formulas test suite (default YES)

8. ``BLACK_TESTS_SAT_BACKEND=<backend>``: backend to use when running tests 


Also, recall some useful standard ``cmake`` options:

1. | ``CMAKE_INSTALL_PREFIX=<path>``: the install prefix used by the ``make 
     install`` command. 
   | Use this option if you do not want to install BLACK system-wise, for 
     example to put it into a local subdirectory of your home directory.

2. | ``CMAKE_BUILD_TYPE=<configuration>``: whether to configure a debug or 
     release build. 
   | Accepted values: ``Release``, ``Debug``, ``RelWithDebInfo`` (default: 
     ``Release``). 
   | ``Debug`` builds run the slowest but build quick and ease
     development. ``Release`` builds are the fastest and are those that should
     be employed by the users. ``RelWithDebInfo`` are optimized like ``Release``
     builds but maintain debug information for easy debugging.