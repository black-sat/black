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
#ifndef declare_hierarchy_element
  #define declare_hierarchy_element(Base, Storage, Element)
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

#ifdef BLACK_DEFINE_TERM_HIERARCHY
  declare_hierarchy(term)
    declare_leaf_storage_kind(term, constant)
      declare_field(term, constant, int, value)
    end_leaf_storage_kind(term, constant)

    declare_leaf_storage_kind(term, variable)
      declare_field(term, variable, identifier, label)
    end_leaf_storage_kind(term, variable)

    declare_leaf_storage_kind(term, application)
      declare_field(term, application, function, func)
      declare_field(term, application, std::vector<term>, terms)
    end_leaf_storage_kind(term, application)

    declare_storage_kind(term, constructor)
      declare_child(term, constructor, argument)
      declare_hierarchy_element(term, constructor, next)
      declare_hierarchy_element(term, constructor, wnext)
      declare_hierarchy_element(term, constructor, prev)
      declare_hierarchy_element(term, constructor, wprev)
    end_storage_kind(term, constructor)
  end_hierarchy(term)
#endif

#ifdef BLACK_DEFINE_FORMULA_HIERARCHY
  declare_hierarchy(formula)

    declare_leaf_storage_kind(formula, boolean)
      declare_field(formula, boolean, bool, value)
    end_leaf_storage_kind(formula, boolean)

    declare_leaf_storage_kind(formula, proposition)
      declare_field(formula, proposition, identifier, label)
    end_leaf_storage_kind(formula, proposition)

    declare_leaf_storage_kind(formula, atom)
      declare_field(formula, atom, identifier, relation)
      declare_field(formula, atom, std::vector<term>, terms)
    end_leaf_storage_kind(formula, atom)

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
#endif 

#undef declare_hierarchy
#undef declare_storage_kind
#undef declare_leaf_storage_kind
#undef declare_field
#undef declare_child
#undef declare_hierarchy_element
#undef end_storage_kind
#undef end_leaf_storage_kind
#undef end_hierarchy
