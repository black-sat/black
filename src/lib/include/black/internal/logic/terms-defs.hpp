//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef declare_ast
  #define declare_ast(NS, AST)
#endif

#ifndef section
  #define section(Doc)
#endif

#ifndef declare_ast_node
  #define declare_ast_node(NS, AST, Node, Doc)
#endif

#ifndef declare_type
  #define declare_type(NS, Decl)
#endif

#ifndef define_type
  #define define_type(NS, Decl, ...)
#endif

#ifndef declare_field
  #define declare_field(NS, AST, Node, Field, Type, Doc)
#endif

#ifndef end_ast_node
  #define end_ast_node(NS, AST, Node)
#endif

#ifndef end_section
  #define end_section()
#endif

#ifndef end_ast
  #define end_ast(NS, AST)
#endif

declare_type(logic, struct entity)

define_type(logic, struct decl,
  struct decl { 
    logic::variable name; 
    logic::types::type type;

    decl(decl const&) = default;
    decl(decl &&) = default;
    
    decl &operator=(decl const&) = default;
    decl &operator=(decl &&) = default;

    decl(variable name, logic::types::type type) 
      : name{name}, type{type} { }

    bool operator==(decl const&) const = default;
    size_t hash() const { return support::hash(name, type); }
  };
)

define_type(logic, struct def,
  struct def {
    variable name;
    types::type type;
    term value;

    def(def const&) = default;
    def(def &&) = default;
    
    def &operator=(def const&) = default;
    def &operator=(def &&) = default;

    def(variable name, types::type type, term value) 
      : name{name.name()}, type{type}, value{value} { }
    
    def(variable name, term value)
      : name{name.name()}, type{types::inferred()}, value{value} { }

    bool operator==(def const&) const = default;
    size_t hash() const { return support::hash(name, type, value); }
  };
)

declare_ast(logic::types, type)

  section("Basic types")
    declare_ast_node(logic::types, type, integer, "The type of integer numbers")
    end_ast_node(logic::types, type, integer)
    
    declare_ast_node(logic::types, type, real, "The type of real numbers")
    end_ast_node(logic::types, type, real)
    
    declare_ast_node(logic::types, type, boolean, "The type of boolean values")
    end_ast_node(logic::types, type, boolean)
    
    declare_ast_node(logic::types, type, function, "The type of functions")
      declare_field(logic::types, type, function, parameters, std::vector<logic::types::type>, "The parameters types")
      declare_field(logic::types, type, function, range, logic::types::type, "The function's range")
    end_ast_node(logic::types, type, function)
  end_section()

  section("Inferred types placeholder")
    declare_ast_node(logic::types, type, inferred, "Placeholder for type inference")
    end_ast_node(logic::types, type, inferred)
  end_section()

  section("The error type")
    declare_ast_node(logic::types, type, error, "A logically erroneous term")
      declare_field(logic::types, type, error, source, logic::term, "The erroneous term")
      declare_field(logic::types, type, error, error, std::string, "The error")
      // TODO: variadic args to format the error
    end_ast_node(logic::types, type, error)
  end_section()

end_ast(logic::types, type)

declare_ast(logic, term)

  section("The error term")
    declare_ast_node(logic, term, error, "A logically erroneous term")
      declare_field(logic, term, error, source, logic::term, "The erroneous term")
      declare_field(logic, term, error, error, std::string, "The error")
      // TODO: variadic args to format the error
    end_ast_node(logic, term, error)
  end_section()

  section("Constant terms")
    declare_ast_node(logic, term, integer, "A constant integer value (e.g., 42)")
      declare_field(logic, term, integer, value, int64_t, "The constant value")
    end_ast_node(logic, term, integer)
    
    declare_ast_node(logic, term, real, "A constant real value (e.g., 3.14)")
      declare_field(logic, term, real, value, double, "The constant value")
    end_ast_node(logic, term, real)

    declare_ast_node(logic, term, boolean, "A constant boolean value (i.e., `true` or `false`)")
      declare_field(logic, term, boolean, value, bool, "The boolean value")
    end_ast_node(logic, term, boolean)
  end_section()

  section("Boolean and first-order predicates")

    declare_ast_node(logic, term, variable, "An unbound variable")
      declare_field(logic, term, variable, name, ast::core::label, "The variable's name")
    end_ast_node(logic, term, variable)
    
    declare_ast_node(logic, term, object, "A resolved object")
      declare_field(logic, term, object, entity, logic::entity const *, "The object's underlying entity")
    end_ast_node(logic, term, object)

    declare_ast_node(logic, term, equal, "An equality constraint between terms")
      declare_field(logic, term, equal, arguments, std::vector<logic::term>, "The operands")
    end_ast_node(logic, term, equal)

    declare_ast_node(logic, term, distinct, "An inequality constraint between terms")
      declare_field(logic, term, distinct, arguments, std::vector<logic::term>, "The operands")
    end_ast_node(logic, term, distinct)
    
    declare_ast_node(logic, term, atom, "An atomic first-order term (e.g. `f(x, y)`)")
      declare_field(logic, term, atom, head, logic::term, "The applied term")
      declare_field(logic, term, atom, arguments, std::vector<logic::term>, "The arguments")
    end_ast_node(logic, term, atom)

    declare_ast_node(logic, term, exists, "An existentially quantified term")
      declare_field(logic, term, exists, binds, std::vector<logic::decl>, "The quantified variables")
      declare_field(logic, term, exists, body, logic::term, "The quantified term")
    end_ast_node(logic, term, exists)

    declare_ast_node(logic, term, forall, "An universally quantified term")
      declare_field(logic, term, forall, binds, std::vector<logic::decl>, "The quantified variables")
      declare_field(logic, term, forall, body, logic::term, "The quantified term")
    end_ast_node(logic, term, forall)

  end_section()

  section("Boolean connectives")
    declare_ast_node(logic, term, negation, "A logical negation")
      declare_field(logic, term, negation, argument, logic::term, "The term to negate")
    end_ast_node(logic, term, negation)
    
    declare_ast_node(logic, term, conjunction, "A logical conjunction")
      declare_field(logic, term, conjunction, arguments, std::vector<logic::term>, "The conjuncts")
    end_ast_node(logic, term, conjunction)
    
    declare_ast_node(logic, term, disjunction, "A logical disjunction")
      declare_field(logic, term, disjunction, arguments, std::vector<logic::term>, "The disjuncts")
    end_ast_node(logic, term, disjunction)
    
    declare_ast_node(logic, term, implication, "A logical implication")
      declare_field(logic, term, implication, left, logic::term, "The antecedent")
      declare_field(logic, term, implication, right, logic::term, "The consequent")
    end_ast_node(logic, term, implication)
  end_section()

  section("Functional constructs")

    declare_ast_node(logic, term, ite, "An if/then/else selection construct")
      declare_field(logic, term, ite, guard, logic::term, "The test guard")
      declare_field(logic, term, ite, iftrue, logic::term, "The result if the guard is true")
      declare_field(logic, term, ite, iffalse, logic::term, "The result if the guard is false")
    end_ast_node(logic, term, ite)

    declare_ast_node(logic, term, lambda, "A lambda abstraction (i.e., an anonymous function)")
      declare_field(logic, term, lambda, vars, std::vector<logic::decl>, "The lambda's parameters")
      declare_field(logic, term, lambda, body, logic::term, "The lambda's body")
    end_ast_node(logic, term, lambda)

    // declare_ast_node(logic, term, placeholder, "A placeholder in a match expression")
    // end_ast_node(logic, term, placeholder)
    
    // declare_ast_node(logic, term, case_of, "A pattern match expression over an ADT")
    //   declare_field(logic, term, case_of, expr, logic::term, "The matched expression")
    //   declare_field(logic, term, case_of, cases, std::vector<logic::pattern>, "The match patterns")
    // end_ast_node(logic, term, case_of)

  end_section()

  section("Linear Temporal Logic (future) temporal operators")

    declare_ast_node(logic, term, tomorrow, "An *tomorrow* LTL formula")
      declare_field(logic, term, tomorrow, argument, logic::term, "The operator's argument")
    end_ast_node(logic, term, tomorrow)

    declare_ast_node(logic, term, w_tomorrow, "A *weak tomorrow* LTL formula")
      declare_field(logic, term, w_tomorrow, argument, logic::term, "The operator's argument")
    end_ast_node(logic, term, w_tomorrow)
    
    declare_ast_node(logic, term, eventually, "An *eventually* LTL formula")
      declare_field(logic, term, eventually, argument, logic::term, "The operator's argument")
    end_ast_node(logic, term, eventually)
    
    declare_ast_node(logic, term, always, "An *always* LTL formula")
      declare_field(logic, term, always, argument, logic::term, "The operator's argument")
    end_ast_node(logic, term, always)
    
    declare_ast_node(logic, term, until, "An *until* LTL formula")
      declare_field(logic, term, until, left, logic::term, "The universal argument")
      declare_field(logic, term, until, right, logic::term, "The existential argument")
    end_ast_node(logic, term, until)
    
    declare_ast_node(logic, term, release, "A *release* LTL formula")
      declare_field(logic, term, release, left, logic::term, "The existential argument")
      declare_field(logic, term, release, right, logic::term, "The universal argument")
    end_ast_node(logic, term, release)

  end_section()

  section("Linear Temporal Logic (past) temporal operators")

    declare_ast_node(logic, term, yesterday, "An *yesterday* LTL formula")
      declare_field(logic, term, yesterday, argument, logic::term, "The operator's argument")
    end_ast_node(logic, term, yesterday)

    declare_ast_node(logic, term, w_yesterday, "A *weak yesterday* LTL formula")
      declare_field(logic, term, w_yesterday, argument, logic::term, "The operator's argument")
    end_ast_node(logic, term, w_yesterday)
    
    declare_ast_node(logic, term, once, "A *once* LTL formula")
      declare_field(logic, term, once, argument, logic::term, "The operator's argument")
    end_ast_node(logic, term, once)
    
    declare_ast_node(logic, term, historically, "An *historically* LTL formula")
      declare_field(logic, term, historically, argument, logic::term, "The operator's argument")
    end_ast_node(logic, term, historically)
    
    declare_ast_node(logic, term, since, "A *since* LTL formula")
      declare_field(logic, term, since, left, logic::term, "The universal argument")
      declare_field(logic, term, since, right, logic::term, "The existential argument")
    end_ast_node(logic, term, since)
    
    declare_ast_node(logic, term, triggered, "A *triggered* LTL formula")
      declare_field(logic, term, triggered, left, logic::term, "The existential argument")
      declare_field(logic, term, triggered, right, logic::term, "The universal argument")
    end_ast_node(logic, term, triggered)

  end_section()

  section("Arithmetic operators")
  
    declare_ast_node(logic, term, minus, "The unary minus")
      declare_field(logic, term, minus, argument, logic::term, "The operand")
    end_ast_node(logic, term, minus)
    
    declare_ast_node(logic, term, sum, "An arithmetic sum")
      declare_field(logic, term, sum, left, logic::term, "The first summand")
      declare_field(logic, term, sum, right, logic::term, "The second summand")
    end_ast_node(logic, term, sum)
    
    declare_ast_node(logic, term, product, "An arithmetic product")
      declare_field(logic, term, product, left, logic::term, "The first factor")
      declare_field(logic, term, product, right, logic::term, "The second factor")
    end_ast_node(logic, term, product)

    declare_ast_node(logic, term, difference, "An arithmetic difference")
      declare_field(logic, term, difference, left, logic::term, "The minuend")
      declare_field(logic, term, difference, right, logic::term, "The subtrahend")
    end_ast_node(logic, term, difference)

    declare_ast_node(logic, term, division, "An arithmetic division")
      declare_field(logic, term, division, left, logic::term, "The numerator")
      declare_field(logic, term, division, right, logic::term, "The denominator")
    end_ast_node(logic, term, division)
  
  end_section()
  
  section("Relational comparisons")
  
    declare_ast_node(logic, term, less_than, "A less-than comparison")
      declare_field(logic, term, less_than, left, logic::term, "The lower argument")
      declare_field(logic, term, less_than, right, logic::term, "The greater argument")
    end_ast_node(logic, term, less_than)
  
    declare_ast_node(logic, term, less_than_eq, "A less-than-or-equal comparison")
      declare_field(logic, term, less_than_eq, left, logic::term, "The lower argument")
      declare_field(logic, term, less_than_eq, right, logic::term, "The greater argument")
    end_ast_node(logic, term, less_than_eq)
  
    declare_ast_node(logic, term, greater_than, "A greater-than comparison")
      declare_field(logic, term, greater_than, left, logic::term, "The greater argument")
      declare_field(logic, term, greater_than, right, logic::term, "The lower argument")
    end_ast_node(logic, term, greater_than)
  
    declare_ast_node(logic, term, greater_than_eq, "A greater-than-or-equal comparison")
      declare_field(logic, term, greater_than_eq, left, logic::term, "The greater argument")
      declare_field(logic, term, greater_than_eq, right, logic::term, "The lower argument")
    end_ast_node(logic, term, greater_than_eq)
  
  end_section()

end_ast(logic, term)

#undef COMMAS
#undef declare_ast
#undef ast_doc
#undef section
#undef declare_ast_node
#undef declare_type
#undef define_type
#undef ast_node_doc
#undef declare_field
#undef end_ast_node
#undef end_section
#undef end_ast