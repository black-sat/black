Manipulation of formulas and terms
====================================

Besides creating logical formulas to be given to solvers, BLACK's API allows us
to *manipulate* such formulas in rich ways, to implement transformations,
encodings, compilations, and more.

Let us suppose the following happens after these declarations and inclusions::

    #include <black/logic>
    #include <cassert>
    
    using namespace black;
    using namespace black::support;
    using namespace black::logic;

Terms and types
----------------

In BLACK, everything is a *term*. Boolean formulas, first-order logic formulas,
numeric expressions, and everything else (except *types*) are different kinds of
terms.

In the API, terms are built by instantiating the corresponding C++ types and
building upon them with operators and functions provided by the API. One of the
simplest terms that one can obtain are instances of :cpp:struct:`object`, which
represent bound names (such as variables declared or defined in a ``module``)::

    module mod;

    object x = mod.declare("x", types::integer()); // see the `Basic usage` page
    object y = mod.declare("y", types::integer());

Now we have two terms ``x`` and ``y``, in particular two objects. Now, we can combine them to obtain more complex terms::

    term expr = x * y + 2; // an expression

    term eq = x + y > 0; // a formula

There is no syntactic distinction between expressions and formulas: both are
terms. However, what distinguish them is the *type*.

BLACK is strictly typed, so each term has a *type*, similarly to many
programming languages, and terms of incompatible types cannot be mixed together.
In the above lines, ``expr`` will have type ``integer`` (following the types of
``x`` and ``y`` and the typing rules of multiplication and addition), while
``eq`` will have type ``boolean`` (following the typing rules of the
greater-than operator). Terms of type ``boolean`` are sometimes called
*formulas*, but they are no different from any other term.

We can also get the type of a term and confirm that what we wrote above is
correct::

    assert(type_of(expr) == types::integer());
    assert(type_of(eq) == types::boolean());

Types and terms are instances of so-called *AST types* (AST stands for *Abstract
Syntax Tree*). AST types are the most general concept handled by BLACK's API,
and many concepts therefore apply both to terms and types. Here we will focus on
how to manipulate terms for simplicity, but most things apply to any other AST
type, including terms, SMT-LIB parsing trees, etc.

Exploring terms
----------------

Terms can be of different alternative kinds. For example, a logical disjunction
is a kind of term, and so are arithmetic additions, and quantified formulas.

A term of any kind can be assigned to a variable of type ``term``, but not *vice
versa*::

    object p = ...;
    object q = ...;

    conjunction f = p && q;

    term t1 = f; // ok
    conjunction g = f; // ok

    conjunction h = t1; // does not compile
    term t2 = p * q; // probably ill-typed, but syntactically ok

This is intuitive because an ``conjunction`` *is a* ``term`` but a ``term`` can
be a lot of things other than an ``conjunction``. 
    
.. note:: 
    Note that all the term types are instantiated and passed around *by value*. 
    That is, we are manipulating the term objects directly, not pointers to 
    them. This avoids many classical C++ memory-management issues.

.. note::
    Although there is a clear subtyping relationship between ``term`` and its 
    different kinds (``object``, ``conjunction``, etc.), this is *not* 
    implemented with a classic C++ object-oriented inheritance scheme, *e.g.* 
    ``conjunction`` is *not* a derived class of ``term``. If this was the case, 
    we would be forced to use *pointers* to term types, conflicting with the 
    previous note.

Given a ``term``, we can understand which kind of term it is by *pattern
matching*. BLACK's API provides a facility that implements *destructuring
pattern matching* for AST types similarly to the feature of many functional
programming languages such as Haskell or Rust. To pattern match on a term, we
use the ``match()`` function. In the following, we implement a function that
returns ``true`` if the given term is a conjunction or a disjunction, and
``false`` otherwise::

    bool is_connective(term t) {
        return match(t)(
            [](conjunction) { return true; },
            [](disjunction) { return true; },
            [](otherwise) { return false; }
        );
    }

There's much to dissect here.

1. The ``match()`` function takes a single argument that is the term to match
   on, and then again another list of parameters which are the handlers for each
   single case. Note the right syntax is ``match(t)(...)``, not ``match(t, ...)``.
2. Each argument of the list is a *C++ lambda function*, which takes as the 
   argument a specific kind of term;
3. The *first* lambda whose argument type matches with the actual type of the
   matched term is called and executed.
4. The return value of the executed lambda is returned by the whole ``match()``
   invocation.

In the specific example above:

1. We give two handlers for the case of the ``conjunction`` and ``disjunction`` 
   types, which return ``true``.
2. ``otherwise`` is an empty type that matches everything, used as a wildcard
   to match anything else;

Let's test the function on the variables declared above::

    assert(is_connective(t1) == true);
    assert(is_connective(t2) == false);

As a more complex example we may want to compute the *depth* of a term. Let us implement such a function focusing for simplicity on a few Boolean connectives::

    int depth(term t) {
        return match(t)(
            [](object) { return 1; },
            [](negation n) {
                return 1 + depth(n.argument());
            },
            [](conjunction c) {
                return 1 + std::max(depth(c.left()), depth(c.right()));
            },
            [](disjunction c) {
                return 1 + std::max(depth(c.left()), depth(c.right()));
            }
        );
    }

Note how the definition of ``depth()`` above corresponds directly to its
straightforward formal definition. The ``match()`` function raises an exception
if the term cannot be handled by the give handlers.

Here, we used member functions ``negation::argument()``,
``conjunction::left()``, ``conjuction::right()`` and so on, to get the
subcomponents of each term. However, we mentioned above that BLACK implements
*destructuring* pattern matching. Indeed, we can destructure terms into their
components while matching on them. This is done by giving additional parameters
to the lambdas, that will directly get the fields of the object as arguments. It
is easier done than said::

    int depth(term t) {
        return match(t)(
            [](object) { return 1; },
            [](negation, term arg) {
                return 1 + depth(arg);
            },
            [](conjunction, term left, term right) {
                return 1 + std::max(depth(left), depth(right));
            },
            [](disjunction, term left, term right) {
                return 1 + std::max(depth(left), depth(right));
            }
        );
    }
