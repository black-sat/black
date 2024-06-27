Basic usage
=====================

BLACK's core API provides facilities to create and manipulate logical entities
such as terms and formulas and solve reasoning tasks over them.

Here we explain the basic concepts of the API usage by writing small pieces of
C++ code that solve simple math problems.

Quadratic equations
~~~~~~~~~~~~~~~~~~~~~

We start by including the relevant headers::
    
    #include <black/logic>
    #include <black/solvers/cvc5>

    #include <cassert> // for the standard `assert` macro

What follows is assumed to go inside our program's ``main()`` function.

Everything in BLACK's API is exposed by the :cpp:any:`black` namespace, and
using the :cpp:any:`black::logic` namespace is also convenient::

    using namespace black;
    using namespace black::logic;

The :cpp:class:`module` class is the entry point for declaring and defining
variables, constants, functions, etc., so we start by declaring an object of
this type::

    module mod;

Within a module we can *declare* or *define* logical entities::

    object x = mod.declare("x", types::real());
    object a = mod.define("a", types::integer(),  1);
    object b = mod.define("b", types::integer(), -8);
    object c = mod.define("c", types::integer(),  16);

Here we have *declared* an object named ``a`` of type ``real`` and defined three
objects, ``a``, ``b``, and ``c``, of type ``integer`` as well. The difference is
the following:

1. a *declared* entity is given a name and a *type*, its value is unknown and 
will have to be found by a solver;

2. a *defined* entity is given a name, a type, and a *value*, therefore its 
value is known and is available to solvers for reasoning.

Modules track a set of *requirements*, that is, assertions that must hold over
the declared and defined objects. Do you remember how to solve quadratic
equations? I do not, so let BLACK help us here::

    mod.require(a * x * x + b * x + c == 0);

Here we are requiring the equation :math:`ax^2 + bx + c = 0` to hold. To
actually solve it we need a *solver*::

    solvers::solver slv = black::solvers::cvc5();

.. note::
    In the current development source tree, only the `cvc5` backend is 
    available, but others will be added soon.

Now we can just ask the solver to check the satisfiability of the module's
requirements::

    support::tribool result = slv.check(mod);

The result is of type :cpp:struct:`support::tribool`, a type that can take three
values: ``true``, ``false``, or ``tribool::undef``. We can check the result is
actually ``true`` because the equation above has solutions::

    assert(result == true);

The equation's polynomial is actually a square, so the equation has a unique
solution which is 4. We can check that the solver agrees::

    assert(slv.value(x) == 4.0);

Note that we had to write ``4.0``, not just ``4``, because ``x`` is of type
``real`` (and ``4.0`` is correspondingly a ``double`` in C++), and ``4`` would
be an ``integer`` (an ``int`` in C++).

.. note::
    Of course, the really useful thing here would be to print the value, 
    because supposedly we did not know it. In the development source tree, 
    printing of terms is still under development.

Are we sure there is a unique solution? Let's find out::

    mod.require(x != 4.0);

    assert(slv.check(mod) == false);

Indeed, if we ask for ``x`` to be anything other than 4, the module's
requirements become unsatisfiable.

Note that we made two seemingly unrelated calls to ``check()``. However, since
at the second call we passed the *same* module, the solver does *not* start from
scratch, but rather reasons incrementally on the new requirement reusing the
work done for the first call as much as possible.

Factorials
~~~~~~~~~~~~~~

What's the number whose factorial is 3628800? Let's find out::

    #include <black/logic>
    #include <black/solvers/cvc5>

    #include <cassert> // for the standard `assert` macro

    using namespace black;
    using namespace black::logic;

As before, we instantiate a module::

    module mod;

Here, we need to define what the *factorial* of a number is. We need to *define*
the factorial function. As you may know, a mathematical definition is the
following:

.. math::
    f(n) = \begin{cases}
        1 & \text{if $n = 1$} \\
        n * f(n - 1) & \text{otherwise}
    \end{cases}

The point here is that this definition is *recursive*. BLACK can handle
recursive function definitions, but we need to define them properly.

In the function's declaration we need to mention two entities: the variable
``n``, and the function itself. But we cannot declare them before declaring the
function itself. So instead we start by instantiating only two `variable`
objects, which stand for two unbound names::

    variable f = "f";
    variable n = "n";

Now we can define the function::

    object fact = mod.define(
        f, {{n, types::integer()}}, types::integer(), 
        ite(n == 1, 1, n * f(n - 1)),
        resolution::delayed
    );

The call to ``define()`` above takes the following arguments:

1. the variable `f` which tells the name of the function;
2. the functions' arguments, as a list of pairs of variables and types; in this case we have only one, ``{{n, types::integer()}}``;
3. the return type of the function, ``types::integer()``;
4. the body of the function: ``ite`` is the *if/then/else* construct, so if ``n`` is 1 the expression is equal to 1, otherwise to ``f(x-1)``;
5. a flag, ``resolution::delayed``, telling BLACK to wait before resolve the scope of the mentioned names.

Without the last flag, the name of `f` would not be found because at this point
is not declared yet. Now we can actually resolve the names, closing the circle::

    mod.resolve(recursion::allowed);

The ``resolve()`` function resolves all the unbound names in the module's
declarations and definitions, and the ``recursion::allowed`` flag tells BLACK
that recursive definitions are allowed.

Now we are ready to answer our original question. So we declare a variable to 
be our answer, and we ask its factorial to be 3628800::

    object x = mod.declare("x", types::integer());

    mod.require(f(x) == 3628800);

We instantiate the solver and check that the requirements are consistent::

    solvers::solver slv = black::solvers::cvc5();

    assert(slv.check(mod) == true);

Somebody told me the answer is 10 (see the note above on why we are not printing
it). Let's check if my source is trustable::

    assert(slv.value(x) == 10);

Everything's worked well!



