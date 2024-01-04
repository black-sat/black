Terms and formulas
====================







``using namespace black::logic;``

.. cpp:namespace:: black::logic



Built-in sorts
~~~~~~~~~~~~~~


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

   
      .. cpp:function:: logic::label name() const

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

   
   
        






