Terms and formulas
====================







``using namespace black::logic;``

.. cpp:namespace:: black::logic



The error term
~~~~~~~~~~~~~~


.. cpp:struct:: error

   A logically erroneous term.

   :constructor:
      .. cpp:function:: error(logic::term source, std::string error)

      
         :param source: The erroneous term.
      
         :param error: The error.
      

      
   :members:

   
      .. cpp:function:: logic::term source() const

         :returns: The erroneous term.

   
      .. cpp:function:: std::string error() const

         :returns: The error.

   
   
        



Sorts and declarations
~~~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: type_type

   The type of types.

   :constructor:
      .. cpp:function:: type_type()

      

   
        

.. cpp:struct:: inferred_type

   A type still to be inferred.

   :constructor:
      .. cpp:function:: inferred_type()

      

   
        

.. cpp:struct:: integer_type

   The type of integer numbers.

   :constructor:
      .. cpp:function:: integer_type()

      

   
        

.. cpp:struct:: real_type

   The type of real numbers.

   :constructor:
      .. cpp:function:: real_type()

      

   
        

.. cpp:struct:: boolean_type

   The type of boolean values.

   :constructor:
      .. cpp:function:: boolean_type()

      

   
        

.. cpp:struct:: function_type

   The type of functions.

   :constructor:
      .. cpp:function:: function_type(std::vector<logic::term> parameters, logic::term range)

      
         :param parameters: The parameters types.
      
         :param range: The function's range.
      

      
   :members:

   
      .. cpp:function:: std::vector<logic::term> parameters() const

         :returns: The parameters types.

   
      .. cpp:function:: logic::term range() const

         :returns: The function's range.

   
   
        

.. cpp:struct:: type_cast

   A type-cast expression.

   :constructor:
      .. cpp:function:: type_cast(logic::term target, logic::term expr)

      
         :param target: The target type.
      
         :param expr: The term to cast.
      

      
   :members:

   
      .. cpp:function:: logic::term target() const

         :returns: The target type.

   
      .. cpp:function:: logic::term expr() const

         :returns: The term to cast.

   
   
        



Constant terms
~~~~~~~~~~~~~~


.. cpp:struct:: integer

   A constant integer value (e.g., 42).

   :constructor:
      .. cpp:function:: integer(int64_t value)

      
         :param value: The constant value.
      

      
   :members:

   
      .. cpp:function:: int64_t value() const

         :returns: The constant value.

   
   
        

.. cpp:struct:: real

   A constant real value (e.g., 3.14).

   :constructor:
      .. cpp:function:: real(double value)

      
         :param value: The constant value.
      

      
   :members:

   
      .. cpp:function:: double value() const

         :returns: The constant value.

   
   
        

.. cpp:struct:: boolean

   A constant boolean value (i.e., `true` or `false`).

   :constructor:
      .. cpp:function:: boolean(bool value)

      
         :param value: The boolean value.
      

      
   :members:

   
      .. cpp:function:: bool value() const

         :returns: The boolean value.

   
   
        



Boolean and first-order predicates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: variable

   An unbound variable.

   :constructor:
      .. cpp:function:: variable(ast::core::label name)

      
         :param name: The variable's name.
      

      
   :members:

   
      .. cpp:function:: ast::core::label name() const

         :returns: The variable's name.

   
   
        

.. cpp:struct:: object

   A resolved object.

   :constructor:
      .. cpp:function:: object(support::toggle_ptr<logic::lookup const> lookup)

      
         :param lookup: The object's lookup info.
      

      
   :members:

   
      .. cpp:function:: support::toggle_ptr<logic::lookup const> lookup() const

         :returns: The object's lookup info.

   
   
        

.. cpp:struct:: equal

   An equality constraint between terms.

   :constructor:
      .. cpp:function:: equal(std::vector<logic::term> arguments)

      
         :param arguments: The operands.
      

      
   :members:

   
      .. cpp:function:: std::vector<logic::term> arguments() const

         :returns: The operands.

   
   
        

.. cpp:struct:: distinct

   An inequality constraint between terms.

   :constructor:
      .. cpp:function:: distinct(std::vector<logic::term> arguments)

      
         :param arguments: The operands.
      

      
   :members:

   
      .. cpp:function:: std::vector<logic::term> arguments() const

         :returns: The operands.

   
   
        

.. cpp:struct:: atom

   An atomic first-order term (e.g. `f(x, y)`).

   :constructor:
      .. cpp:function:: atom(logic::term head, std::vector<logic::term> arguments)

      
         :param head: The applied term.
      
         :param arguments: The arguments.
      

      
   :members:

   
      .. cpp:function:: logic::term head() const

         :returns: The applied term.

   
      .. cpp:function:: std::vector<logic::term> arguments() const

         :returns: The arguments.

   
   
        

.. cpp:struct:: exists

   An existentially quantified term.

   :constructor:
      .. cpp:function:: exists(std::vector<logic::decl> binds, logic::term body)

      
         :param binds: The quantified variables.
      
         :param body: The quantified term.
      

      
   :members:

   
      .. cpp:function:: std::vector<logic::decl> binds() const

         :returns: The quantified variables.

   
      .. cpp:function:: logic::term body() const

         :returns: The quantified term.

   
   
        

.. cpp:struct:: forall

   An universally quantified term.

   :constructor:
      .. cpp:function:: forall(std::vector<logic::decl> binds, logic::term body)

      
         :param binds: The quantified variables.
      
         :param body: The quantified term.
      

      
   :members:

   
      .. cpp:function:: std::vector<logic::decl> binds() const

         :returns: The quantified variables.

   
      .. cpp:function:: logic::term body() const

         :returns: The quantified term.

   
   
        



Boolean connectives
~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: negation

   A logical negation.

   :constructor:
      .. cpp:function:: negation(logic::term argument)

      
         :param argument: The term to negate.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The term to negate.

   
   
        

.. cpp:struct:: conjunction

   A logical conjunction.

   :constructor:
      .. cpp:function:: conjunction(std::vector<logic::term> arguments)

      
         :param arguments: The conjuncts.
      

      
   :members:

   
      .. cpp:function:: std::vector<logic::term> arguments() const

         :returns: The conjuncts.

   
   
        

.. cpp:struct:: disjunction

   A logical disjunction.

   :constructor:
      .. cpp:function:: disjunction(std::vector<logic::term> arguments)

      
         :param arguments: The disjuncts.
      

      
   :members:

   
      .. cpp:function:: std::vector<logic::term> arguments() const

         :returns: The disjuncts.

   
   
        

.. cpp:struct:: implication

   A logical implication.

   :constructor:
      .. cpp:function:: implication(logic::term left, logic::term right)

      
         :param left: The antecedent.
      
         :param right: The consequent.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The antecedent.

   
      .. cpp:function:: logic::term right() const

         :returns: The consequent.

   
   
        



Functional constructs
~~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: ite

   An if/then/else selection construct.

   :constructor:
      .. cpp:function:: ite(logic::term guard, logic::term iftrue, logic::term iffalse)

      
         :param guard: The test guard.
      
         :param iftrue: The result if the guard is true.
      
         :param iffalse: The result if the guard is false.
      

      
   :members:

   
      .. cpp:function:: logic::term guard() const

         :returns: The test guard.

   
      .. cpp:function:: logic::term iftrue() const

         :returns: The result if the guard is true.

   
      .. cpp:function:: logic::term iffalse() const

         :returns: The result if the guard is false.

   
   
        

.. cpp:struct:: lambda

   A lambda abstraction (i.e., an anonymous function).

   :constructor:
      .. cpp:function:: lambda(std::vector<logic::decl> vars, logic::term body)

      
         :param vars: The lambda's parameters.
      
         :param body: The lambda's body.
      

      
   :members:

   
      .. cpp:function:: std::vector<logic::decl> vars() const

         :returns: The lambda's parameters.

   
      .. cpp:function:: logic::term body() const

         :returns: The lambda's body.

   
   
        



Linear Temporal Logic (future) temporal operators
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: tomorrow

   An *tomorrow* LTL formula.

   :constructor:
      .. cpp:function:: tomorrow(logic::term argument)

      
         :param argument: The operator's argument.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The operator's argument.

   
   
        

.. cpp:struct:: w_tomorrow

   A *weak tomorrow* LTL formula.

   :constructor:
      .. cpp:function:: w_tomorrow(logic::term argument)

      
         :param argument: The operator's argument.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The operator's argument.

   
   
        

.. cpp:struct:: eventually

   An *eventually* LTL formula.

   :constructor:
      .. cpp:function:: eventually(logic::term argument)

      
         :param argument: The operator's argument.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The operator's argument.

   
   
        

.. cpp:struct:: always

   An *always* LTL formula.

   :constructor:
      .. cpp:function:: always(logic::term argument)

      
         :param argument: The operator's argument.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The operator's argument.

   
   
        

.. cpp:struct:: until

   An *until* LTL formula.

   :constructor:
      .. cpp:function:: until(logic::term left, logic::term right)

      
         :param left: The universal argument.
      
         :param right: The existential argument.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The universal argument.

   
      .. cpp:function:: logic::term right() const

         :returns: The existential argument.

   
   
        

.. cpp:struct:: release

   A *release* LTL formula.

   :constructor:
      .. cpp:function:: release(logic::term left, logic::term right)

      
         :param left: The existential argument.
      
         :param right: The universal argument.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The existential argument.

   
      .. cpp:function:: logic::term right() const

         :returns: The universal argument.

   
   
        



Linear Temporal Logic (past) temporal operators
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: yesterday

   An *yesterday* LTL formula.

   :constructor:
      .. cpp:function:: yesterday(logic::term argument)

      
         :param argument: The operator's argument.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The operator's argument.

   
   
        

.. cpp:struct:: w_yesterday

   A *weak yesterday* LTL formula.

   :constructor:
      .. cpp:function:: w_yesterday(logic::term argument)

      
         :param argument: The operator's argument.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The operator's argument.

   
   
        

.. cpp:struct:: once

   A *once* LTL formula.

   :constructor:
      .. cpp:function:: once(logic::term argument)

      
         :param argument: The operator's argument.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The operator's argument.

   
   
        

.. cpp:struct:: historically

   An *historically* LTL formula.

   :constructor:
      .. cpp:function:: historically(logic::term argument)

      
         :param argument: The operator's argument.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The operator's argument.

   
   
        

.. cpp:struct:: since

   A *since* LTL formula.

   :constructor:
      .. cpp:function:: since(logic::term left, logic::term right)

      
         :param left: The universal argument.
      
         :param right: The existential argument.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The universal argument.

   
      .. cpp:function:: logic::term right() const

         :returns: The existential argument.

   
   
        

.. cpp:struct:: triggered

   A *triggered* LTL formula.

   :constructor:
      .. cpp:function:: triggered(logic::term left, logic::term right)

      
         :param left: The existential argument.
      
         :param right: The universal argument.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The existential argument.

   
      .. cpp:function:: logic::term right() const

         :returns: The universal argument.

   
   
        



Arithmetic operators
~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: minus

   The unary minus.

   :constructor:
      .. cpp:function:: minus(logic::term argument)

      
         :param argument: The operand.
      

      
   :members:

   
      .. cpp:function:: logic::term argument() const

         :returns: The operand.

   
   
        

.. cpp:struct:: sum

   An arithmetic sum.

   :constructor:
      .. cpp:function:: sum(logic::term left, logic::term right)

      
         :param left: The first summand.
      
         :param right: The second summand.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The first summand.

   
      .. cpp:function:: logic::term right() const

         :returns: The second summand.

   
   
        

.. cpp:struct:: product

   An arithmetic product.

   :constructor:
      .. cpp:function:: product(logic::term left, logic::term right)

      
         :param left: The first factor.
      
         :param right: The second factor.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The first factor.

   
      .. cpp:function:: logic::term right() const

         :returns: The second factor.

   
   
        

.. cpp:struct:: difference

   An arithmetic difference.

   :constructor:
      .. cpp:function:: difference(logic::term left, logic::term right)

      
         :param left: The minuend.
      
         :param right: The subtrahend.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The minuend.

   
      .. cpp:function:: logic::term right() const

         :returns: The subtrahend.

   
   
        

.. cpp:struct:: division

   An arithmetic division.

   :constructor:
      .. cpp:function:: division(logic::term left, logic::term right)

      
         :param left: The numerator.
      
         :param right: The denominator.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The numerator.

   
      .. cpp:function:: logic::term right() const

         :returns: The denominator.

   
   
        



Relational comparisons
~~~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: less_than

   A less-than comparison.

   :constructor:
      .. cpp:function:: less_than(logic::term left, logic::term right)

      
         :param left: The lower argument.
      
         :param right: The greater argument.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The lower argument.

   
      .. cpp:function:: logic::term right() const

         :returns: The greater argument.

   
   
        

.. cpp:struct:: less_than_eq

   A less-than-or-equal comparison.

   :constructor:
      .. cpp:function:: less_than_eq(logic::term left, logic::term right)

      
         :param left: The lower argument.
      
         :param right: The greater argument.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The lower argument.

   
      .. cpp:function:: logic::term right() const

         :returns: The greater argument.

   
   
        

.. cpp:struct:: greater_than

   A greater-than comparison.

   :constructor:
      .. cpp:function:: greater_than(logic::term left, logic::term right)

      
         :param left: The greater argument.
      
         :param right: The lower argument.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The greater argument.

   
      .. cpp:function:: logic::term right() const

         :returns: The lower argument.

   
   
        

.. cpp:struct:: greater_than_eq

   A greater-than-or-equal comparison.

   :constructor:
      .. cpp:function:: greater_than_eq(logic::term left, logic::term right)

      
         :param left: The greater argument.
      
         :param right: The lower argument.
      

      
   :members:

   
      .. cpp:function:: logic::term left() const

         :returns: The greater argument.

   
      .. cpp:function:: logic::term right() const

         :returns: The lower argument.

   
   
        






