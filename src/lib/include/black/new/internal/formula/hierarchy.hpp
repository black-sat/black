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
#ifndef has_no_standard_equality
  #define has_no_standard_equality(Base)
#endif
#ifndef declare_storage_kind
  #define declare_storage_kind(Base, Storage)
#endif
#ifndef declare_leaf_storage_kind
  #define declare_leaf_storage_kind declare_storage_kind
#endif
#ifndef declare_storage_custom_members
  #define declare_storage_custom_members(Base, Storage, Struct)
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
#ifndef escape_commas
#define escape_commas(...) __VA_ARGS__
#endif 

declare_hierarchy(symbol)
  declare_leaf_storage_kind(symbol, relation)
    declare_storage_custom_members(symbol, relation, relation_call_op)
    declare_field(symbol, relation, identifier, label)
  end_leaf_storage_kind(symbol, relation)
  declare_leaf_storage_kind(symbol, function)
    declare_storage_custom_members(symbol, function, function_call_op)
    declare_field(symbol, function, identifier, label)
  end_leaf_storage_kind(symbol, function)
end_hierarchy(symbol)

declare_hierarchy(number)
  declare_storage_kind(number, identity)
    declare_leaf_hierarchy_element(number, identity, zero)
    declare_leaf_hierarchy_element(number, identity, one)
  end_storage_kind(number, identity)
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
    declare_field(term, application, function, func)
    declare_children(term, application, term, terms)
    has_no_hierarchy_elements(term, application)
  end_storage_kind(term, application)

  declare_storage_kind(term, unary_term)
    declare_child(term, unary_term, term, argument)
    declare_hierarchy_element(term, unary_term, negative)
    declare_hierarchy_element(term, unary_term, next)
    declare_hierarchy_element(term, unary_term, wnext)
    declare_hierarchy_element(term, unary_term, prev)
    declare_hierarchy_element(term, unary_term, wprev)
  end_storage_kind(term, unary_term)

  declare_storage_kind(term, binary_term)
    declare_child(term, binary_term, term, left)
    declare_child(term, binary_term, term, right)
    declare_hierarchy_element(term, binary_term, addition)
    declare_hierarchy_element(term, binary_term, subtraction)
    declare_hierarchy_element(term, binary_term, multiplication)
    declare_hierarchy_element(term, binary_term, division)
  end_storage_kind(term, binary_term)
end_hierarchy(term)

declare_hierarchy(formula)

  declare_leaf_storage_kind(formula, boolean)
    declare_field(formula, boolean, bool, value)
  end_leaf_storage_kind(formula, boolean)

  declare_leaf_storage_kind(formula, proposition)
    declare_field(formula, proposition, identifier, label)
  end_leaf_storage_kind(formula, proposition)

  declare_storage_kind(formula, atom)
    declare_field(formula, atom, relation, rel)
    declare_children(formula, atom, term, terms)
    has_no_hierarchy_elements(formula, atom)
  end_storage_kind(formula, atom)

  declare_storage_kind(formula, comparison)
    declare_child(formula, comparison, term, left)
    declare_child(formula, comparison, term, right)
    declare_hierarchy_element(formula, comparison, equal)
    declare_hierarchy_element(formula, comparison, not_equal)
    declare_hierarchy_element(formula, comparison, less_than)
    declare_hierarchy_element(formula, comparison, less_than_equal)
    declare_hierarchy_element(formula, comparison, greater_than)
    declare_hierarchy_element(formula, comparison, greater_than_equal)
  end_storage_kind(formula, comparison)

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

declare_hierarchy(sort)
  declare_leaf_storage_kind(sort, custom_sort)
    declare_field(sort, custom_sort, identifier, name)
  end_leaf_storage_kind(sort, custom_sort)
  declare_storage_kind(sort, primitive_sort)
    declare_leaf_hierarchy_element(sort, primitive_sort, integer_sort)
    declare_leaf_hierarchy_element(sort, primitive_sort, real_sort)
  end_storage_kind(sort, primitive_sort)
  declare_storage_kind(sort, function_sort)
    declare_field(sort, function_sort, identifier, name)
    declare_child(sort, function_sort, sort, return_sort)
    declare_children(sort, function_sort, sort, arguments)
    has_no_hierarchy_elements(sort, function_sort)
  end_storage_kind(sort, function_sort)
  declare_storage_kind(sort, relation_sort)
    declare_field(sort, relation_sort, identifier, name)
    declare_children(sort, relation_sort, sort, arguments)
    has_no_hierarchy_elements(sort, relation_sort)
  end_storage_kind(sort, relation_sort)
end_hierarchy(sort)

#undef declare_hierarchy
#undef has_no_standard_equality
#undef declare_storage_kind
#undef declare_leaf_storage_kind
#undef declare_storage_custom_members
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
#undef escape_commas
