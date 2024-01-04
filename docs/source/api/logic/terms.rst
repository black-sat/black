Terms and formulas
====================







``using namespace black::logic;``

.. cpp:namespace:: black::logic



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

   A constant boolean value (i.e. `true` or `false`).

   :constructor:
      .. cpp:function:: boolean(bool value)

      
         :param value: The boolean value.
      

   :members:

   
      .. cpp:function:: bool value() const

         :returns: The boolean value.

   
        



Boolean and first-order predicates
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. cpp:struct:: symbol

   A named symbol (e.g. a variable or a predicate).

   :constructor:
      .. cpp:function:: symbol(logic::label name)

      
         :param name: The symbol's label.
      

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
      .. cpp:function:: negation(logic::term operand)

      
         :param operand: The term to negate.
      

   :members:

   
      .. cpp:function:: logic::term operand() const

         :returns: The term to negate.

   
        

.. cpp:struct:: conjunction

   A logical conjunction.

   :constructor:
      .. cpp:function:: conjunction(std::vector<logic::term> operands)

      
         :param operands: The conjuncts.
      

   :members:

   
      .. cpp:function:: std::vector<logic::term> operands() const

         :returns: The conjuncts.

   
        

.. cpp:struct:: disjunction

   A logical disjunction.

   :constructor:
      .. cpp:function:: disjunction(std::vector<logic::term> operands)

      
         :param operands: The disjuncts.
      

   :members:

   
      .. cpp:function:: std::vector<logic::term> operands() const

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

   
        






