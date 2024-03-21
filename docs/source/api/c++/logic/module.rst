Module system
===============

The `module` class is the entry point for building modules to be analysed or
solved.

.. cpp:namespace:: black::logic

.. doxygenclass:: black::logic::module

.. cpp:struct:: decl

      The specification of an entity to be declared in a module.

      .. rubric:: Constructors

      .. cpp:function:: inline decl(variable name, term type)
      
         Constructs the object.

      .. rubric:: Public Members

      .. cpp:member:: variable name

         The name of the entity.
      
      .. cpp:member:: term type

         The type of the entity.

.. doxygenstruct:: black::logic::def
.. doxygenstruct:: black::logic::function_def
.. doxygenenum:: black::logic::resolution
.. doxygenenum:: black::logic::scope
.. doxygenstruct:: black::logic::lookup