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

#ifndef declare_ast_factory
  #define declare_ast_factory(NS, AST, Factory, Member)
#endif

#ifndef declare_ast_node
  #define declare_ast_node(NS, AST, Node, Doc)
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

declare_ast(logic, term)

  declare_ast_factory(logic, term, alphabet, sigma)

  section("Constant terms")
    declare_ast_node(logic, term, integer, "A constant integer value (e.g., 42)")
      declare_field(logic, term, integer, value, int64_t, "The constant value")
    end_ast_node(logic, term, integer)
    
    declare_ast_node(logic, term, real, "A constant real value (e.g., 3.14)")
      declare_field(logic, term, real, value, double, "The constant value")
    end_ast_node(logic, term, real)

    declare_ast_node(logic, term, boolean, "A constant boolean value (i.e. `true` or `false`)")
      declare_field(logic, term, boolean, value, bool, "The boolean value")
    end_ast_node(logic, term, boolean)
  end_section()

  section("Boolean and first-order predicates")

    declare_ast_node(logic, term, symbol, "A named symbol (e.g. a variable or a predicate)")
      declare_field(logic, term, symbol, name, logic::label, "The symbol's label")
    end_ast_node(logic, term, symbol)

    declare_ast_node(logic, term, equal, "An equality constraint between terms")
      declare_field(logic, term, equal, arguments, std::vector<logic::term>, "The operands")
    end_ast_node(logic, term, symbol)

    declare_ast_node(logic, term, distinct, "An inequality constraint between terms")
      declare_field(logic, term, distinct, arguments, std::vector<logic::term>, "The operands")
    end_ast_node(logic, term, symbol)
    
    declare_ast_node(logic, term, atom, "An atomic first-order term (e.g. `f(x, y)`)")
      declare_field(logic, term, atom, head, logic::term, "The applied symbol")
      declare_field(logic, term, atom, arguments, std::vector<logic::term>, "The arguments")
    end_ast_node(logic, term, symbol)

  end_section()

  section("Boolean connectives")
    declare_ast_node(logic, term, negation, "A logical negation")
      declare_field(logic, term, negation, operand, logic::term, "The term to negate")
    end_ast_node(logic, term, negation)
    
    declare_ast_node(logic, term, conjunction, "A logical conjunction")
      declare_field(logic, term, conjunction, operands, std::vector<logic::term>, "The conjuncts")
    end_ast_node(logic, term, conjunction)
    
    declare_ast_node(logic, term, disjunction, "A logical disjunction")
      declare_field(logic, term, disjunction, operands, std::vector<logic::term>, "The disjuncts")
    end_ast_node(logic, term, disjunction)
    
    declare_ast_node(logic, term, implication, "A logical implication")
      declare_field(logic, term, implication, left, logic::term, "The antecedent")
      declare_field(logic, term, implication, right, logic::term, "The consequent")
    end_ast_node(logic, term, implication)
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

end_ast(logic, term)


#undef declare_ast
#undef ast_doc
#undef section
#undef declare_ast_factory
#undef declare_ast_node
#undef ast_node_doc
#undef declare_field
#undef end_ast_node
#undef end_section
#undef end_ast