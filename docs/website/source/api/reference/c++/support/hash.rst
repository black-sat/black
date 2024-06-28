Hashing utilities
==================

.. cpp:namespace:: black::support

.. cpp:concept:: template<typename T> hashable

   Concept describing types on which `std::hash` is applicable.

.. cpp:function:: template<> size_t hash(hashable ...args)
.. cpp:function:: template<std::ranges::range R> size_t hash(R const& arg)
.. cpp:function:: template<TupleLike T> size_t hash(T const& arg)

   A wrapper around `std::hash` that computes the hash of a value or a sequence of values, taken from, respectively:

   1. a variadic list of :cpp:any:`hashable` arguments
   2. a range of :cpp:any:`hashable` types
   3. a tuple-like object whose components are :cpp:any:`hashable`