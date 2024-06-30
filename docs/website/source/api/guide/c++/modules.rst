Modules
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
   the ``resolution::delayed`` flag to delay the resolution of names; the
   variable is used both as the name for the definition and inside the
   definition's body itself;
3. calling the :cpp:func:`resolve()<black::logic::module::resolve>` member
   function with the ``recursion::allowed`` flag to perform name resolution on
   the newly defined entity.

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
``resolution::delayed`` flag and resolved with a single call to
:cpp:func:`resolve()<black::logic::module::resolve>`. Entities declared or
defined with ``resolution::delayed`` are refered to as *pending*. Pending
entities will not be visible in name lookup until the next call to
:cpp:func:`resolve()<black::logic::module::resolve>`, when all unbound names
used in the body of all pending definitions will be resolved by name lookup.
However, name lookup is performed in two different ways depending on the flag
passed to :cpp:func:`resolve()<black::logic::module::resolve>`:

1. if ``recursion::forbidden`` is used, name lookup will *not* see the current 
   pending entities, so recursive definitions are not possible;
2. if ``recursion::allowed`` is used, name lookup will see all the current
   pending entities, independently from the order they have been
   declared/defined.

In particular, in the second case, the independence from the
declaration/definition order means that mutually recursive definitions are also
possible::
    
    module mod;

    variable f = "f", g = "g", x = "x";
    
    mod.define(f, {{x, types::integer()}}, types::integer(), g(x));
    mod.define(g, {{x, types::integer()}}, types::integer(), f(x));

    mod.resolve(recursion::allowed); // `f` will see `g` and `g` will see `f`


This mechanism allows us to implement different kinds of surface languages using
a single tool:

1. for languages with sequential scoping, such as SMT-LIB, where any entity must
   be declared before being used, we can declare/define each entity by itself
   with the ``resolution::immediate`` flag (the default);
2. for languages with mutually recursive scoping (e.g. the BLACK modeling
   language, still under development), or for the `define-funs-rec` directive of
   SMT-LIB, we can define the entities in groups using the
   ``resolution::delayed`` flag and resolve them together with a single call to
   :cpp:func:`resolve()<black::logic::module::resolve>`.

Representing reactive systems
-------------------------------

Conceptually, a module represents a synchronous reactive system described
symbolically by means of logical sentences, whose signature is made of the
entities declared and defined in the module.

By default, declared entities are considered *rigid*, that is, arbitrary but
constant in time, and detached from the dynamic behavior of the module. This
corresponds to passing the ``role::rigid`` flag, which is the default, to the
:cpp:func:`declare()<black::logic::module::declare>` function::

    module mod;

    // the following lines are equivalent
    mod.declare("x", types::integer(), role::rigid); 
    mod.declare("x", types::integer());    

Entities can be otherwise be considered as *flowing*, that is, changing over
time. Flowing entities can be of the following kinds:

1. input entities (``role::input``) are controlled by the environment outside of
   the system; the system's behavior can observe them but not control their
   value;
2. output entities (``role::output``) are controlled by the system, which can
   arbitrarily decide their value at each execution step;
3. state entities (``role::state``) represent the internal state of the system,
   which is unknown to outside components and possibly changes at each execution
   step depending on the system's transition relation;

The dynamic behavior of the system is defined by means of the following
components:

1. an *initial condition* that describes the valid configurations of the state
   entities at the start of the system's execution;
2. a *final condition* that describes the configurations of the state entities
   where the system's execution is allowed to stop;
3. a *transition relation* that describes how state entities and output entities
   evolve based on the previous state and possibly the input entities;

The following example defines a transition system with a Boolean state
variable, an integer input, and an integer output, where at each step the
output mirrors the input in even steps and negates it in odd steps::

    module mod;

    mod.declare("p", types::boolean(), role::state);
    mod.declare("x", types::integer(), role::input);
    mod.declare("y", types::integer(), role::output);

    mod.init(p == true);
    mod.transition(
        (p && X(!p) && y = x) || (!p && X(p) && y = -x)
    );

Requirements can talk about input and output entities (but not about state
entities), and work as the public specification of the module. The following
states that in any step the output is either equal to the input or to its
opposite::

    mod.require(G(y == x || y == -x));

.. note::
    The model-checking solver (under development) will be able to confirm that 
    the specification is valid over the system.

Names and labels
-----------------

All the examples so far have used strings to give names to variables and
declared/defined entities, but this is not required. In each place where a name
is expected (such as in calls to the
:cpp:func:`define()<black::logic::module::define>` and
:cpp:func:`declare()<black::logic::module::declare>` functions, and in
definition of functions, quantifiers, etc.), the name is given by a
:cpp:type:`variable<black::logic::variable>` object, whose name is given by
objects of type :cpp:type:`label<black::logic::label>`. 

Labels are a powerful concept in the day-to-day usage of BLACK's API. Their key
feature is that they can be constructed by any *hashable regular type*. This
means almost any value type in C++ type system that implements a specialization
of :cpp:type:`std::hash`. This include strings, as used in previous examples,
but also any instance of such types.

The most important use case of the flexibility of labels is when many entities
have to be defined iteratively keeping some sort of index in their names. In the
API of most SMT solvers this requires messing with string formatting, but not
here. For example, let us define a bus of input Boolean variables, whose width
is given by the variable :cpp:var:`N`::

    module mod;
    for(size_t i = 0; i < N; i++)
        mod.declare(std::pair{"bus", i}, types::boolean(), role::input);

Now, each variable of the bus can be retrieved by name lookup using
``std::pair{"bus", i}`` as name (or we may have saved each object returned by
:cpp:func:`declare()<black::logic::module::declare>` in a vector).


Memory management
----------------------

:cpp:type:`object<black::logic::object>` instances returned by
:cpp:func:`define()<black::logic::module::define>` and
:cpp:func:`declare()<black::logic::module::declare>` point to instances of type
:cpp:struct:`entity<black::logic::entity>`, which are the actual objects
representing the declared/defined entity. These objects are constant and their
lifetime is managed by the :cpp:any:`module<black::logic::module>` class. 

As a general rule, objects and their underlying
:cpp:struct:`entity<black::logic::entity>` instances are kept alive as long as
the :cpp:any:`module<black::logic::module>` instance they come from is alive.
The lifetime of entities is managed in groups, corresponding to each call of
:cpp:func:`resolve()<black::logic::module::resolve>` (which is implicit when
``resolution::immediate`` is used), represented by instance of type
:cpp:struct:`root<black::logic::root>`. Roots can be shared between different
modules. A module can *adopt* at once all the entities managed by a root, using
the :cpp:func:`adopt()<black::logic::module::adopt>` member function. Objects,
roots, and entities are constant objects, so they can be shared freely between
different modules without issues.