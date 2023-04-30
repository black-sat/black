Assertion macros
====================

.. c:macro:: black_assert(Expr)

   Asserts that `Expr` must evaluate to `true`.

   :param Expr: `bool` expression that must evaluate to `true`.
   :throws: :cpp:class:`assert_error` if `Expr` is `false`.

.. c:macro:: black_unreachable()

   Specifies that the current line of code is unreachable.

   :throws: :cpp:class:`unreachable_error`

.. c:macro:: black_assume(Expr, Loc, Message)
   
   Specifies that the current function assumes that `Expr` evaluates 
   to `true`.

   :param Expr: `bool` expression that is assumed to be `true`
   :param Loc: :cpp:type:`source_location` value of the function
   :param Message: `const char *` message to show in case of failure
   :throws: :cpp:class:`assume_error` in case of failure