Terms and formulas
====================







``using namespace black::logic;``

.. cpp:namespace:: black::logic



Sorts and declarations
~~~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: sort_sort

   The sort of sorts.

   
   .. note::
      This term type is *primitive*, therefore it has no 
      constructors. Use :cpp:func:`alphabet::sort_sort()` to 
      create objects of this type.
   

   
        

.. cpp:struct:: integer_sort

   The sort of integer numbers.

   
   .. note::
      This term type is *primitive*, therefore it has no 
      constructors. Use :cpp:func:`alphabet::integer_sort()` to 
      create objects of this type.
   

   
        

.. cpp:struct:: real_sort

   The sort of real numbers.

   
   .. note::
      This term type is *primitive*, therefore it has no 
      constructors. Use :cpp:func:`alphabet::real_sort()` to 
      create objects of this type.
   

   
        

.. cpp:struct:: boolean_sort

   The sort of boolean values.

   
   .. note::
      This term type is *primitive*, therefore it has no 
      constructors. Use :cpp:func:`alphabet::boolean_sort()` to 
      create objects of this type.
   

   
        

.. cpp:struct:: function_sort

   The sort of functions.

   
   :constructor:
      .. cpp:function:: function_sort(logic::term range, std::vector<logic::term> arguments)

      
         :param range: The function's range.
      
         :param arguments: The function's arguments.
      

   

      
   :members:

   
      .. cpp:function:: logic::term range() const

         :returns: The function's range.

   
      .. cpp:function:: std::vector<logic::term> arguments() const

         :returns: The function's arguments.

   
   
        

.. cpp:struct:: decl

   A declaration.

   
   :constructor:
      .. cpp:function:: decl(logic::symbol name, logic::term sort)

      
         :param name: The declared symbol.
      
         :param sort: The sort of the declared symbol.
      

   

      
   :members:

   
      .. cpp:function:: logic::symbol name() const

         :returns: The declared symbol.

   
      .. cpp:function:: logic::term sort() const

         :returns: The sort of the declared symbol.

   
   
        

.. cpp:struct:: cast

   A type-cast expression.

   
   :constructor:
      .. cpp:function:: cast(logic::term target, logic::term expr)

      
         :param target: The target sort.
      
         :param expr: The term to cast.
      

   

      
   :members:

   
      .. cpp:function:: logic::term target() const

         :returns: The target sort.

   
      .. cpp:function:: logic::term expr() const

         :returns: The term to cast.

   
   
        



Constant terms
~~~~~~~~~~~~~~


.. cpp:struct:: integer

   A constant integer value (e.g., 42).

   
   .. note::
      This term type is *primitive*, therefore it has no 
      constructors. Use :cpp:func:`alphabet::integer()` to 
      create objects of this type.
   

      
   :members:

   
      .. cpp:function:: int64_t value() const

         :returns: The constant value.

   
   
        

.. cpp:struct:: real

   A constant real value (e.g., 3.14).

   
   .. note::
      This term type is *primitive*, therefore it has no 
      constructors. Use :cpp:func:`alphabet::real()` to 
      create objects of this type.
   

      
   :members:

   
      .. cpp:function:: double value() const

         :returns: The constant value.

   
   
        

.. cpp:struct:: boolean

   A constant boolean value (i.e., `true` or `false`).

   
   .. note::
      This term type is *primitive*, therefore it has no 
      constructors. Use :cpp:func:`alphabet::boolean()` to 
      create objects of this type.
   

      
   :members:

   
      .. cpp:function:: bool value() const

         :returns: The boolean value.

   
   
        



Boolean and first-order predicates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: symbol

   A named symbol (e.g., a variable or a predicate).

   
   .. note::
      This term type is *primitive*, therefore it has no 
      constructors. Use :cpp:func:`alphabet::symbol()` to 
      create objects of this type.
   

      
   :members:

   
      .. cpp:function:: std::string name() const

         :returns: The symbol's label.

   
   
        

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

      
         :param head: The applied symbol.
      
         :param arguments: The arguments.
      

   

      
   :members:

   
      .. cpp:function:: logic::term head() const

         :returns: The applied symbol.

   
      .. cpp:function:: std::vector<logic::term> arguments() const

         :returns: The arguments.

   
   
        



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

   A lambda abstraction.

   
   :constructor:
      .. cpp:function:: lambda(std::vector<logic::decl> vars, logic::term body)

      
         :param vars: The abstracted variables.
      
         :param body: The lambda's body.
      

   

      
   :members:

   
      .. cpp:function:: std::vector<logic::decl> vars() const

         :returns: The abstracted variables.

   
      .. cpp:function:: logic::term body() const

         :returns: The lambda's body.

   
   
        

.. cpp:struct:: placeholder

   A placeholder in a match expression.

   
   .. note::
      This term type is *primitive*, therefore it has no 
      constructors. Use :cpp:func:`alphabet::placeholder()` to 
      create objects of this type.
   

   
        

.. cpp:struct:: pattern

   A pattern in a match expression.

   
   :constructor:
      .. cpp:function:: pattern(logic::term head, logic::term body)

      
         :param head: The pattern's head.
      
         :param body: The pattern's body.
      

   

      
   :members:

   
      .. cpp:function:: logic::term head() const

         :returns: The pattern's head.

   
      .. cpp:function:: logic::term body() const

         :returns: The pattern's body.

   
   
        

.. cpp:struct:: caseof

   A pattern match expression over an ADT.

   
   :constructor:
      .. cpp:function:: caseof(logic::term expr, std::vector<logic::pattern> cases)

      
         :param expr: The matched expression.
      
         :param cases: The match patterns.
      

   

      
   :members:

   
      .. cpp:function:: logic::term expr() const

         :returns: The matched expression.

   
      .. cpp:function:: std::vector<logic::pattern> cases() const

         :returns: The match patterns.

   
   
        



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
      .. cpp:function:: sum(std::vector<logic::term> arguments)

      
         :param arguments: The addends.
      

   

      
   :members:

   
      .. cpp:function:: std::vector<logic::term> arguments() const

         :returns: The addends.

   
   
        

.. cpp:struct:: product

   An arithmetic product.

   
   :constructor:
      .. cpp:function:: product(std::vector<logic::term> arguments)

      
         :param arguments: The factors.
      

   

      
   :members:

   
      .. cpp:function:: std::vector<logic::term> arguments() const

         :returns: The factors.

   
   
        

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

   
   
        






