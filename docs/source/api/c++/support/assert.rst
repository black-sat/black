Assertion macros
====================

.. cpp:namespace:: black::support

.. c:macro:: BLACK_ASSERT_DISABLE

   If defined, disables the :c:macro:`black_assert` macro.

.. c:macro:: black_assert(Expr)

   Asserts that `Expr` must evaluate to `true`.

   :param Expr: `bool` expression that must evaluate to `true`.
   :throws: :cpp:class:`bad_assert` if `Expr` is `false`.

.. c:macro:: black_unreachable()

   Specifies that the current line of code is unreachable.

   :throws: :cpp:class:`bad_unreachable`

.. c:macro:: black_assume(Expr, Loc, Message)
   
   Specifies that the current function assumes that `Expr` evaluates 
   to `true`.

   :param Expr: `bool` expression that is assumed to be `true`
   :param Loc: :cpp:type:`std::source_location` value of the function
   :param Message: `const char *` message to show in case of failure
   :throws: :cpp:class:`bad_assumption` in case of failure