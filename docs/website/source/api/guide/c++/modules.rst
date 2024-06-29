Module system
===============

.. namespace:: black::logic

BLACK provides a rich and flexible type system to represent reactive systems and
constraints over them. The core component of BLACK's API is the
:cpp:any:`module<black::logic::module>` class, which allows us to declare and
define logical entities (variables, constants, function, etc.), state
requirements and different kind of constraints, and manipulate them. The
:cpp:any:`module<black::logic::module>` class also takes care of scoping and
symbol lookup, which are crucial in implementing modeling languages such as
SMT-LIB an MoXI.

The following happens after these inclusions and declarations::

    #include <black/logic>

    using namespace black;
    using namespace black::logic;

Declarations and definitions
------------------------------

BLACK modules, similarly to SMT-LIB, distinguish between *declared* and
*defined* entities. We have already mentioned the difference between these two
concepts in the :ref:`guide_basics` page. To recap:

1. *declared* entities are known to the module but their *value* is unknown and
   supposed to be found by a solver;
2. *defined* entities are also given a known value, possibly defined on top of 
   other entities;

Entities are declared and defined using the
:cpp:func:`declare()<black::logic::module::declare>` and
:cpp:func:`define()<black::logic::module::define>` member functions. Both
return an object of type :cpp:type:`object<black::logic::object>`, which
represent the entity as known to the module.
:cpp:type:`object<black::logic::object>` instances are terms, and can be used to
build more complex terms on top of them::

    module mod;

    object x = mod.declare("x", types::integer());
    object y = mod.declare("y", types::integer());

    term f = x + y;

Entities can also be retrieved by name, using the
:cpp:func:`lookup()<black::logic::module::lookup>` member function, which again
returns the corresponding :cpp:type:`object<black::logic::object>` wrapped in an
`std::optional` which is empty if the name is not found::

    std::optional<object> optx = mod.lookup("x");
    // use `optx` if not empty...

Recursive definitions
----------------------

As seen in the :ref:`guide_basics` page, BLACK supports the definition of
recursive functions. This is done in three steps:

1. declaring an instance of the `variable` type, which represents the 
   function's unbound name;
2. calling the :cpp:func:`define()<black::logic::module::define>` function using
   the `resolution::delayed` flag to delay the resolution of names; the variable
   is used both as the name for the definition and inside the definition's body
   itself;
3. calling the :cpp:func:`resolve()<black::logic::module::resolve>` member
   function with the `recursion::allowed` flag to perform name resolution on the
   newly defined entity.

Borrowing the example from :ref:`guide_basics`::

    variable f = "f";
    variable n = "n";

    object fact = mod.define(
        f, {{n, types::integer()}}, types::integer(), 
        ite(n == 1, 1, n * f(n - 1)),
        resolution::delayed
    );

    mod.resolve(recursion::allowed);

This mechanism supports much more than simple recursive definitions. An
*arbitrary* number of entities can be declared or defined with the
`resolution::delayed` flag and resolved with a single call to
:cpp:func:`resolve()<black::logic::module::resolve>`. Entities declared or
defined with `resolution::delayed` are refered to as *pending*. Pending entities
will not be visible in name lookup until the next call to
:cpp:func:`resolve()<black::logic::module::resolve>`, when all unbound names
used in the body of all pending definitions will be resolved by name lookup.
However, name lookup is performed in two different ways depending on the flag
passed to :cpp:func:`resolve()<black::logic::module::resolve>`:

1. if `recursion::forbidden` is used, name lookup will *not* see the current 
   pending entities, so recursive definitions are not possible;
2. if `recursion::allowed` is used, name lookup will see all the current pending
   entities, independently from the order they have been declared/defined.

In particular, in the second case, the independence from the
declaration/definition order means that mutually recursive definitions are also
possible::
    
    module mod;

    variable f = "f", g = "g", x = "x";
    
    mod.define(f, {{x, types::integer()}}, types::integer(), g(x));
    mod.define(g, {{x, types::integer()}}, types::integer(), f(x));

    mod.resolve(recursion::allowed); // `f` will see `g` and `g` will see `f`


This mechanism allows us to implement different kind of surface languages using
a single tool:

1. for languages with sequential scoping, such as SMT-LIB, where any entity must
   be declared before being used, we can declare/define each entity by itself
   with the `resolution::immediate` flag (the default);
2. for languages with mutually recursive scoping (e.g. the BLACK modeling
   language, still under development), or for the `define-funs-rec` directive of
   SMT-LIB, we can define the entities in groups using the `resolution::delayed`
   flag and resolve them together with a single call to
   :cpp:func:`resolve()<black::logic::module::resolve>`.