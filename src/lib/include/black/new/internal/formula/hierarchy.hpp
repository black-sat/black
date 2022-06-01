//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

//
// Note: no include guards. This file is designed to be included many times.
//

#ifndef declare_hierarchy
  #define declare_hierarchy(Base)
#endif
#ifndef declare_storage_kind
  #define declare_storage_kind(Base, Storage)
#endif
#ifndef declare_leaf_storage_kind
  #define declare_leaf_storage_kind declare_storage_kind
#endif
#ifndef declare_field
  #define declare_field(Base, Storage, Type, Field)
#endif
#ifndef declare_child
  #define declare_child(Base, Storage, Child)
#endif
#ifndef has_no_hierarchy_elements
  #define has_no_hierarchy_elements(Base, Storage)
#endif
#ifndef declare_hierarchy_element
  #define declare_hierarchy_element(Base, Storage, Element)
#endif
#ifndef declare_leaf_hierarchy_element
  #define declare_leaf_hierarchy_element declare_hierarchy_element
#endif
#ifndef end_storage_kind
  #define end_storage_kind(Base, Storage)
#endif
#ifndef end_leaf_storage_kind
  #define end_leaf_storage_kind end_storage_kind
#endif
#ifndef end_hierarchy
  #define end_hierarchy(Element)
#endif
#ifndef declare_fragment
  #define declare_fragment(Fragment, Namespace)
#endif
#ifndef end_fragment
  #define end_fragment(Fragment, Namespace)
#endif
#ifndef escape_commas
#define escape_commas(...) __VA_ARGS__
#endif 

declare_hierarchy(symbol)
  declare_leaf_storage_kind(symbol, uninterpreted)
    declare_field(symbol, uninterpreted, identifier, label)
  end_leaf_storage_kind(symbol, uninterpreted)
  declare_storage_kind(symbol, function)
    declare_leaf_hierarchy_element(symbol, function, negative)
    declare_leaf_hierarchy_element(symbol, function, subtraction)
    declare_leaf_hierarchy_element(symbol, function, addition)
    declare_leaf_hierarchy_element(symbol, function, multiplication)
    declare_leaf_hierarchy_element(symbol, function, division)
  end_storage_kind(symbol, function)
  declare_storage_kind(symbol, relation)
    declare_leaf_hierarchy_element(symbol, relation, equal)
    declare_leaf_hierarchy_element(symbol, relation, not_equal)
    declare_leaf_hierarchy_element(symbol, relation, less_than)
    declare_leaf_hierarchy_element(symbol, relation, less_than_equal)
    declare_leaf_hierarchy_element(symbol, relation, greater_than)
    declare_leaf_hierarchy_element(symbol, relation, greater_than_equal)
  end_storage_kind(symbol, relation)
end_hierarchy(symbol)

declare_hierarchy(term)
  declare_leaf_storage_kind(term, constant)
    declare_field(term, constant, int, value)
  end_leaf_storage_kind(term, constant)

  declare_leaf_storage_kind(term, variable)
    declare_field(term, variable, identifier, label)
  end_leaf_storage_kind(term, variable)

  declare_storage_kind(term, compound)
    declare_field(
      term, compound, 
      escape_commas(symbol<syntax<
        uninterpreted,
        negative,
        subtraction,
        addition,
        multiplication,
        division
      >>),
      func
    )
    // declare_field(term, compound, std::vector<term<Syntax>>, terms)
    has_no_hierarchy_elements(term, compound)
  end_storage_kind(term, compound)

  declare_storage_kind(term, constructor)
    declare_child(term, constructor, argument)
    declare_hierarchy_element(term, constructor, next)
    declare_hierarchy_element(term, constructor, wnext)
    declare_hierarchy_element(term, constructor, prev)
    declare_hierarchy_element(term, constructor, wprev)
  end_storage_kind(term, constructor)
end_hierarchy(term)

declare_hierarchy(formula)

  declare_leaf_storage_kind(formula, boolean)
    declare_field(formula, boolean, bool, value)
  end_leaf_storage_kind(formula, boolean)

  declare_leaf_storage_kind(formula, proposition)
    declare_field(formula, proposition, identifier, label)
  end_leaf_storage_kind(formula, proposition)

  declare_storage_kind(formula, atom)
    declare_field(
      formula, atom, 
      escape_commas(symbol<syntax<
        uninterpreted,
        equal,
        not_equal,
        less_than,
        less_than_equal,
        greater_than,
        greater_than_equal
      >>), 
      rel
    )
    // declare_field(formula, atom, std::vector<term<Syntax>>, terms)
    has_no_hierarchy_elements(formula, atom)
  end_storage_kind(formula, atom)

  declare_storage_kind(formula, quantifier)
    declare_field(formula, quantifier, variable, var)
    declare_child(formula, quantifier, matrix)
    declare_hierarchy_element(formula, quantifier, exists)
    declare_hierarchy_element(formula, quantifier, forall)
  end_storage_kind(formula, quantifier)
  
  declare_storage_kind(formula, unary)
    declare_child(formula, unary, argument)
    declare_hierarchy_element(formula, unary, negation)
    declare_hierarchy_element(formula, unary, tomorrow)
    declare_hierarchy_element(formula, unary, w_tomorrow)
    declare_hierarchy_element(formula, unary, yesterday)
    declare_hierarchy_element(formula, unary, w_yesterday)
    declare_hierarchy_element(formula, unary, always)
    declare_hierarchy_element(formula, unary, eventually)
    declare_hierarchy_element(formula, unary, once)
    declare_hierarchy_element(formula, unary, historically)
  end_storage_kind(formula, unary)

  declare_storage_kind(formula, binary)
    declare_child(formula, binary, left)
    declare_child(formula, binary, right)
    declare_hierarchy_element(formula, binary, conjunction)
    declare_hierarchy_element(formula, binary, disjunction)
    declare_hierarchy_element(formula, binary, implication)
    declare_hierarchy_element(formula, binary, iff)
    declare_hierarchy_element(formula, binary, until)
    declare_hierarchy_element(formula, binary, release)
    declare_hierarchy_element(formula, binary, w_until)
    declare_hierarchy_element(formula, binary, s_release)
    declare_hierarchy_element(formula, binary, since)
    declare_hierarchy_element(formula, binary, triggered)
  end_storage_kind(formula, binary)

end_hierarchy(formula)

declare_fragment(Boolean, boolean)
  include_leaf_storage_kind(Boolean, boolean)
  include_leaf_storage_kind(Boolean, proposition)
  include_storage_kind(Boolean, unary)
    include_hierarchy_element(Boolean, unary, negation)
  end_include_storage_kind(Boolean, unary)
  include_storage_kind(Boolean, binary)
    include_hierarchy_element(Boolean, binary, conjunction)
    include_hierarchy_element(Boolean, binary, disjunction)
    include_hierarchy_element(Boolean, binary, implication)
    include_hierarchy_element(Boolean, binary, iff)
  end_include_storage_kind(Boolean, binary)
end_fragment(Boolean, boolean)

declare_derived_fragment(FO, fo, Boolean)
  // formulas 
  include_storage_kind(FO, atom)
    include_no_hierarchy_elements(FO, atom)
  end_include_storage_kind()
  include_storage_kind(FO, quantifier)
    include_hierarchy_element(FO, quantifier, exists)
    include_hierarchy_element(FO, quantifier, forall)
  end_include_storage_kind()
  
  // terms
  include_leaf_storage_kind(FO, constant)
  include_leaf_storage_kind(FO, variable)
  include_storage_kind(FO, compound)
    include_no_hierarchy_elements(FO, compound)
  end_include_storage_kind(FO, compound)
  
  // functions and relations
  include_leaf_storage_kind(FO, uninterpreted)
  include_storage_kind(symbol, function)
    include_hierarchy_element(FO, negative)
    include_hierarchy_element(FO, subtraction)
    include_hierarchy_element(FO, addition)
    include_hierarchy_element(FO, multiplication)
    include_hierarchy_element(FO, division)
  end_include_storage_kind(symbol, function)
  include_storage_kind(symbol, relation)
    include_hierarchy_element(FO, equal)
    include_hierarchy_element(FO, not_equal)
    include_hierarchy_element(FO, less_than)
    include_hierarchy_element(FO, less_than_equal)
    include_hierarchy_element(FO, greater_than)
    include_hierarchy_element(FO, greater_than_equal)
  end_include_storage_kind(symbol, relation)
end_derived_fragment(FO, boolean)

declare_derived_fragment(LTL, ltl, Boolean)
  include_storage_kind(LTL, unary)
    include_hierarchy_element(LTL, tomorrow)
    include_hierarchy_element(LTL, w_tomorrow)
    include_hierarchy_element(LTL, always)
    include_hierarchy_element(LTL, eventually)
  end_include_storage_kind(LTL, unary)
  include_storage_kind(LTL, binary)
    include_hierarchy_element(LTL, until)
    include_hierarchy_element(LTL, release)
    include_hierarchy_element(LTL, w_until)
    include_hierarchy_element(LTL, s_release)
  end_include_storage_kind(LTL, binary)
end_derived_fragment(LTL, ltl, Boolean)

declare_derived_fragment(LTLP, ltlp, LTL)
  include_storage_kind(LTLP, unary)
    include_hierarchy_element(LTLP, yesterday)
    include_hierarchy_element(LTLP, w_yesterday)
    include_hierarchy_element(LTLP, once)
    include_hierarchy_element(LTLP, historically)
  end_include_storage_kind(LTLP, unary)
  include_storage_kind(LTLP, binary)
    include_hierarchy_element(LTLP, since)
    include_hierarchy_element(LTLP, triggered)
  end_include_storage_kind(LTLP, binary)
end_derived_fragment(LTLP, ltlp, LTL)

declare_combined_fragment(LTLFO, ltlfo, LTL, FO)

#undef declare_hierarchy
#undef declare_storage_kind
#undef declare_leaf_storage_kind
#undef declare_field
#undef declare_child
#undef has_no_leaf_hierarchy_elements
#undef has_no_hierarchy_elements
#undef declare_leaf_hierarchy_element
#undef declare_hierarchy_element
#undef end_storage_kind
#undef end_leaf_storage_kind
#undef end_hierarchy
#undef declare_fragment
#undef end_fragment
#undef escape_commas