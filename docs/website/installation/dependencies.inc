To compile BLACK you need the following packages:
   
1. `CMake`_ 3.25 or above
2. `libfmt`_ 7.x or above
3. `Hopscotch-map`_ 2.x
4. `nlohmann-json`_ 3.5 or above

Possible backends (of which at least one must be present):

1. `Z3`_ 4.8.x or above
2. `cvc5`_ 1.2.1 or above (not cvc4)
3. `MathSAT`_ 5.6.7 or later
4. `CryptoMiniSAT`_ 5.x 

The MathSAT and MiniSAT backends require `zlib`_ 1.x. MathSAT also requires
`gmp`_ 6.x and its C++ wrapper.

You can choose any of these backends to compile and use BLACK. Note however that
not all the backends support the same features. In particular, Z3, cvc5 or
MathSAT are required to handle *first-order* formulas, and only Z3 and cvc5
support *quantified* first-order formulas.

.. _CMake: https://cmake.org
.. _libfmt: https://github.com/fmtlib/fmt
.. _Hopscotch-map: https://github.com/Tessil/hopscotch-map
.. _nlohmann-json: https://github.com/nlohmann/json
.. _CryptoMiniSAT: https://github.com/msoos/cryptominisat
.. _Z3: https://github.com/Z3Prover/z3
.. _cvc5: https://cvc5.github.io
.. _MathSAT: http://mathsat.fbk.eu
.. _zlib: https://zlib.net/
.. _gmp: https://gmplib.org/