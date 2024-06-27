Pipeline stages
================

Some simple utility pipeline stages are predefined here. All pipeline stages are
implemented as `inline constexpr` *callable* function objects with a 
:cpp:type:`transform::pipeline` return type, that for readability here are documented as
functions.

.. cpp:namespace:: black::pipes

.. cpp:function:: transform::pipeline id();

   Does nothing and replays its source *as-is*.

.. cpp:function:: 
      transform::pipeline \
      composed(transform::pipeline first, transform::pipeline second);

   Composes `first` and `second` into a single pipeline stage. This is also
   callable with an overloaded *bitwise or* operation (the pipe symbol) as
   follows:

   .. code-block:: cpp

      transform::pipeline pipe = id() | map(...);
   

.. cpp:function:: 
      transform::pipeline \
      map( \
         std::function<type(type)> types_map, \
         std::function<term(term)> terms_map, \
         std::function<term(term)> back_map \
      );

   Maps `types_map` and `terms_map` to any type or term, respectively, appearing
   in the source declarations/definitions/statements. Uses `back_map` to
   implement :cpp:func:`transform::base::undo()`.