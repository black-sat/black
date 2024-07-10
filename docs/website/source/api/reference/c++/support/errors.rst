Error types
============

The following types form a taxonomy of all the possible errors that BLACK can
return *to the user*. These types are **not** thrown as exceptions, but returned
as the error component of a :cpp:class:`std::expected` object. This 
ensures they are always handled explicitly.

.. cpp:namespace:: black::support

.. doxygenstruct:: black::support::error

.. doxygenstruct:: black::support::syntax_error

.. doxygenstruct:: black::support::io_error

