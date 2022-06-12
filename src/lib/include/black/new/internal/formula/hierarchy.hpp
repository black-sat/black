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
#ifndef declare_nonfragmented_hierarchy
  #define declare_nonfragmented_hierarchy declare_hierarchy
#endif
#ifndef has_no_standard_equality
  #define has_no_standard_equality(Base)
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
  #define declare_child(Base, Storage, Hierarchy, Child)
#endif
#ifndef declare_children
  #define declare_children(Base, Storage, Hierarchy, Children)
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
#ifndef end_nonfragmented_hierarchy
  #define end_nonfragmented_hierarchy end_hierarchy
#endif
#ifndef escape_commas
#define escape_commas(...) __VA_ARGS__
#endif 

declare_hierarchy(function)
  declare_leaf_storage_kind(function, function_symbol)
    declare_field(function, function_symbol, identifier, label)
  end_leaf_storage_kind(function, function_symbol)
  declare_storage_kind(function, known_func)
    declare_leaf_hierarchy_element(function, known_func, negative)
    declare_leaf_hierarchy_element(function, known_func, subtraction)
    declare_leaf_hierarchy_element(function, known_func, addition)
    declare_leaf_hierarchy_element(function, known_func, multiplication)
    declare_leaf_hierarchy_element(function, known_func, division)
  end_storage_kind(function, known_func)
end_hierarchy(function)

declare_hierarchy(relation)
  declare_leaf_storage_kind(relation, relation_symbol)
    declare_field(relation, relation_symbol, identifier, label)
  end_leaf_storage_kind(relation, relation_symbol)
  declare_storage_kind(relation, known_rel)
    declare_leaf_hierarchy_element(relation, known_rel, equal)
    declare_leaf_hierarchy_element(relation, known_rel, not_equal)
    declare_leaf_hierarchy_element(relation, known_rel, less_than)
    declare_leaf_hierarchy_element(relation, known_rel, less_than_equal)
    declare_leaf_hierarchy_element(relation, known_rel, greater_than)
    declare_leaf_hierarchy_element(relation, known_rel, greater_than_equal)
  end_storage_kind(relation, known_rel)
end_hierarchy(relation)

declare_hierarchy(number)
  declare_leaf_storage_kind(number, integer)
    declare_field(number, integer, int64_t, value)
  end_leaf_storage_kind(number, integer)
  declare_leaf_storage_kind(number, real)
    declare_field(number, real, double, value)
  end_leaf_storage_kind(number, real)
end_hierarchy(number)

declare_hierarchy(term)
  has_no_standard_equality(term)
  declare_storage_kind(term, constant)
    declare_child(term, constant, number, value)
    has_no_hierarchy_elements(term, constant)
  end_storage_kind(term, constant)

  declare_leaf_storage_kind(term, variable)
    declare_field(term, variable, identifier, label)
  end_leaf_storage_kind(term, variable)

  declare_storage_kind(term, application)
    declare_child(term, application, function, func)
    declare_children(term, application, term, terms)
    has_no_hierarchy_elements(term, application)
  end_storage_kind(term, application)

  declare_storage_kind(term, constructor)
    declare_child(term, constructor, term, argument)
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
    declare_child(formula, atom, relation, rel)
    declare_children(formula, atom, term, terms)
    has_no_hierarchy_elements(formula, atom)
  end_storage_kind(formula, atom)

  declare_storage_kind(formula, quantifier)
    declare_field(formula, quantifier, variable, var)
    declare_child(formula, quantifier, formula, matrix)
    declare_hierarchy_element(formula, quantifier, exists)
    declare_hierarchy_element(formula, quantifier, forall)
  end_storage_kind(formula, quantifier)
  
  declare_storage_kind(formula, unary)
    declare_child(formula, unary, formula, argument)
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
    declare_child(formula, binary, formula, left)
    declare_child(formula, binary, formula, right)
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

declare_nonfragmented_hierarchy(sort)
  declare_leaf_storage_kind(sort, custom_sort)
    declare_field(sort, custom_sort, identifier, name)
  end_leaf_storage_kind(sort, custom_sort)
end_nonfragmented_hierarchy(sort)

#undef declare_hierarchy
#undef declare_nonfragmented_hierarchy
#undef has_no_standard_equality
#undef declare_storage_kind
#undef declare_leaf_storage_kind
#undef declare_field
#undef declare_child
#undef declare_children
#undef has_no_leaf_hierarchy_elements
#undef has_no_hierarchy_elements
#undef declare_leaf_hierarchy_element
#undef declare_hierarchy_element
#undef end_storage_kind
#undef end_leaf_storage_kind
#undef end_hierarchy
#undef end_nonfragmented_hierarchy
#undef escape_commas
