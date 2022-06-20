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
// This is the hierarchy definition file. Through a number of macros (that are
// *not* defined here) we define the syntactic structure of the available
// formulas, terms, etc... This file is thought to be included multiple times.
// By defining each time in a different way the macros, one can create pretty
// complex structures based on the hierarchy defined here. See `generation.hpp`
// to see the actual code generation happening. Here, we comment the meaning of
// these macros from a modeling perspective. Together they form a
// domain-specific language to define ADT-like hierarchies of types.
//
// This file is structured in three parts:
// - the first part defines all the macros as empty, *if* they are not defined.
//   This means that when including this file, only the single macros needed for
//   that specific inclusion have to be defined. Some macros, such as
//   `declare_leaf_storage_kind`, are defined to be equivalent to others (i.e.
//   `declare_storage_kind` in this case), when they are not defined. This
//   allows us to use general definitions when sufficient (i.e. for any storage
//   kind) and go in detail when needed.
// - the second part is the hierarchy definition itself, see later for a
//   description.
// - the third part undefines all the macros after their use. This allows the
//   including file to forget about them as soon as the #include <> directive is
//   issued. Otherwise, the caller would be responsible to undef the defined
//   macros before the next inclusion. 
//
// The hierarchy definition needs some terminology to be understood. This
// terminology is similar to the one used in `core.hpp` and other places.
// - a `hierarchy` is a top-level type which can be thought as the base class of
//   all its subtypes. `formula`, `term`, etc... are hierarchies.
// - a `storage kind` is a subtype of a given hierarchy that has a specific
//   layout: a specific number of fields and children with their type. It's
//   called in this way because the layout of storage kinds define how the
//   `alphabet` class will store them in its internal structures. For example,
//   the `unary` and `binary` types are storage kinds of the `formula`
//   hierarchy. `unary` defines its layout to only have a single child called
//   `argument`, while `binary` defines it to have two children called `left`
//   and `right`. Storage kinds can, at run-time, be of different types. They
//   always store their type (as a value of the `syntax_element` enum), so it is
//   not listed here among the fields.
// - a `hierarchy element` is the bottom level of the hierarchy, corresponding
//   to a specific type of a storage kind. For example, `conjunction` and
//   `disjunction` are two hierachy elements of the `binary` storage kind. They
//   share all the fields and children from their parent storage kind, with the
//   only difference being that the type is fixed and known at compile-time.
// - a `field` is a data attribute of a storage kind (and of its hierarchy
//   elements). For example, the `boolean` storage kind has the field `value`
//   which is of type `bool`. This results into an actual member function called
//   `value()` exposed by the class `boolean`.
// - a `child` is a data attribute of a storage kind whose type is a hierarchy
//   hierarchy and thus is subject to the same restrictions regarding its syntax
//   as imposed by the fragment applied to the current storage kind. For
//   example, `binary` has two children `left` and `right` of `formula`
//   hierarchy. This results into two actual member functions called `left()`
//   and `right()` exposed by the class `binary<Syntax>`, for some fragment
//   `Syntax`, which return values of type `formula<Syntax>`.
// - a `children vector` is a collection of a dynamic number of children of the
//   same hierarchy. For example, the `atom` storage kind has a children vector
//   `terms` of hierarchy `term`. This results into the class `atom<Syntax>`,
//   for some fragment `Syntax`, to expose a member function called `terms()`
//   which returns a range of elements of type `term<Syntax>`.
// - a "leaf" entity is one that has no children and no hierarchy elements. As
//   such, the corresponding type will be non-templated. For example, `boolean`
//   is a leaf storage kind, since it only has a field `bool value` but no
//   children and no hierarchy elements. The class `boolean` is concrete, not a
//   template, since there are no children to constrain the fragment of. `zero`
//   is a leaf hierarchy element of the storage kind `identity` (hierarchy
//   `number`). It has to be, because its parent storage kind defines no
//   children.
// - `simple` hierarchies and `simple` storage kinds are entities that are not
//   parameterized over the fragment, but are always used with the universal
//   fragment. An example is the `sort` hierarchy type and its storage kinds.
//   Simple hierarchies are limited in their use. They cannot appear as children
//   of other hierarchy types (but they can appear as fields). They cannot have
//   children, but only fields, and consequently they cannot have hierarchy
//   elements but only leaf hierarchy elements.

// The macros used to define the entities listed above have a certain common
// structure. Each macro, excepting for the top-level `declare_hierarchy`, has
// to be put inside the pair `declare`/`end` of its parent macro, e.g.
// `declare_storage_kind()` must appear inside a pair of
// `declare_hierarchy()`/`end_hierarchy()`. Each invocation of a macro repeats
// all the arguments of its parent macros. So for example, inside the call to
// `declare_hierarchy(formula)`, we have the call to
// `declare_storage_kind(formula, binary)`, and inside that we have
// `declare_hierarchy_element(formula, binary, conjunction)`. This is a bit
// repetitive but there is no other way to ensure that the expansion of each 
// macro has all the context to know what to do. Some macros are tought to be 
// used in pairs, e.g. `declare_hierarchy`/`end_hierarchy`. In this case, the 
// `end_` macro takes always the same arguments as the `declare_` one.
//
// The actual macros are the following:
//
// - declare_hierarchy(Base)/end_hierarchy 
//     Declare a hierarchy. 
//     - Arguments:
//       - `Base` is the name of the hierarchy (e.g. `formula`).
//
// - declare_simple_hierarch(Base)/end_simple_hierarchy
//     Declare a simple hierarchy. 
//     - Arguments:
//       - `Base` is the name of the simple hierarchy (e.g. `sort`).
//
// - declare_storage_kind(Base, Storage)/end_storage_kind 
//     Declare a storage kind. 
//     - Parent macro: `declare_hierarchy` 
//     - Arguments:
//       - `Base` is the name of the parent hierarchy
//       - `Storage` is the name of the storage kind 
//
// - declare_simple_storage_kind(Base, Storage)/end_simple_storage_kind 
//     Declare a simple storage kind. 
//     - Parent macro: `declare_simple_hierarchy` 
//     - Arguments:
//       - `Base` is the name of the parent simple hierarchy
//       - `Storage` is the name of the simple storage kind 
//
// - declare_leaf_storage_kind(Base, Storage)/end_leaf_storage_kind 
//     Declare a leaf storage kind. 
//     - Parent macro: `declare_hierarchy` or `declare_simple_hierarchy`
//     - Arguments:
//       - `Base` is the name of the parent hierarchy
//       - `Storage` is the name of the storage kind 
//
// - declare_storage_custom_members(Base, Storage, Struct) 
//     Declare a custom type declaring custom members for the given storage
//     kind. The given identifier must name a template class taking a single
//     type template parameter. This template class will be used as a CRTP base
//     class for `storage_base` in `core.hpp`, thus providing additional custom
//     members for all storage types and hierarchy elements of that type. 
//     - Parent macro: 
//        `declare_storage_kind`, `declare_leaf_storage_kind` or 
//        `declare_leaf_storage_kind`
//     - Arguments:
//       - `Base` is the name of the parent hierarchy
//       - `Storage` is the name of the parent storage kind 
//       - `Struct` is the name of the custom members class
//
// - declare_field(Base, Storage, Type, Field) 
//     Declare a member field of a storage kind. 
//     - Parent macro: 
//        `declare_storage_kind`, `declare_leaf_storage_kind` or 
//        `declare_leaf_storage_kind`
//     - Arguments:
//       - `Base` is the name of the parent hierarchy
//       - `Storage` is the name of the parent storage kind 
//       - `Type` is the type of the field
//       - `Field` is the name of the field
//
// - declare_child(Base, Storage, Hierarchy, Child) 
//     Declare a child of a storage kind. 
//     - Parent macro: `declare_storage_kind`
//     - Arguments:
//       - `Base` is the name of the parent hierarchy
//       - `Storage` is the name of the parent storage kind 
//       - `Hierarchy` is the hierarchy to which the child belongs
//       - `Child`  is the name of the resulting field
//
// - declare_children(Base, Storage, Hierarchy, Children) 
//     Declare a children vector for a storage kind. It must appear at most
//     once for a given storage kind. 
//     - Parent macro: `declare_storage_kind`
//     - Arguments:
//       - `Base` is the name of the parent hierarchy
//       - `Storage` is the name of the parent storage kind 
//       - `Hierarchy` is the hierarchy to which the children belong
//       - `Children` is the name of the resulting field
//
// - declare_hierarchy_element(Base, Storage, Element) 
//     Declares a hierarchy element for a given storage kind. At least one of 
//     this, `declare_leaf_hierarchy_element`, or `has_no_hierarchy_elements` 
//     must appear in a given storage kind. 
//     - Parent macro: `declare_storage_kind`
//     - Arguments:
//       - `Base` is the name of the parent hierarchy
//       - `Storage` is the name of the parent storage kind 
//       - `Element` is the name of the element
//
// - declare_leaf_hierarchy_element(Base, Storage, Element) 
//     Declares a hierarchy element for a given storage kind. At least one of 
//     this, `declare_hierarchy_element`, or `has_no_hierarchy_elements` must 
//     appear in a given storage kind. If this macro appears in a storage kind, 
//     the latter cannot declare children.
//     - Parent macro: `declare_storage_kind`
//     - Arguments:
//       - `Base` is the name of the parent hierarchy
//       - `Storage` is the name of the parent storage kind 
//       - `Element` is the name of the element
//
// - has_no_hierarchy_elements(Base, Storage) 
//     States that the enclosing storage kind has no hierarchy elements. At 
//     least one of this, `declare_hierarchy_element`, or 
//     `declare_leaf_hierarchy_element` must appear in a given (non-leaf) 
//     storage kind. It must not appear together with 
//     `declare_leaf_hierarchy_element` or `declare_hierarchy_element`. 
//     - Parent macro: `declare_storage_kind`
//     - Arguments:
//       - `Base` is the name of the parent hierarchy
//       - `Storage` is the name of the parent storage kind 
//

#ifndef declare_hierarchy
  #define declare_hierarchy(Base)
#endif
#ifndef declare_simple_hierarchy
  #define declare_simple_hierarchy declare_hierarchy
#endif
#ifndef has_no_standard_equality
  #define has_no_standard_equality(Base)
#endif
#ifndef declare_storage_kind
  #define declare_storage_kind(Base, Storage)
#endif
#ifndef declare_simple_storage_kind
  #define declare_simple_storage_kind declare_storage_kind
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
#ifndef end_simple_storage_kind
  #define end_simple_storage_kind end_storage_kind
#endif
#ifndef end_leaf_storage_kind
  #define end_leaf_storage_kind end_storage_kind
#endif
#ifndef end_hierarchy
  #define end_hierarchy(Element)
#endif
#ifndef end_simple_hierarchy
  #define end_simple_hierarchy end_hierarchy
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

declare_simple_hierarchy(sort)
  declare_leaf_storage_kind(sort, custom_sort)
    declare_field(sort, custom_sort, identifier, name)
  end_leaf_storage_kind(sort, custom_sort)
  declare_simple_storage_kind(sort, primitive_sort)
    declare_leaf_hierarchy_element(sort, primitive_sort, integer_sort)
    declare_leaf_hierarchy_element(sort, primitive_sort, real_sort)
  end_simple_storage_kind(sort, primitive_sort)
  declare_leaf_storage_kind(sort, function_sort)
    declare_field(sort, function_sort, identifier, name)
    declare_field(sort, function_sort, sort, return_sort)
    declare_field(sort, function_sort, std::vector<sort>, arguments)
  end_leaf_storage_kind(sort, function_sort)
  declare_leaf_storage_kind(sort, relation_sort)
    declare_field(sort, relation_sort, identifier, name)
    declare_field(sort, relation_sort, std::vector<sort>, arguments)
  end_leaf_storage_kind(sort, relation_sort)
end_simple_hierarchy(sort)

#undef declare_hierarchy
#undef declare_simple_hierarchy
#undef has_no_standard_equality
#undef declare_storage_kind
#undef declare_simple_storage_kind
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
#undef end_simple_storage_kind
#undef end_leaf_storage_kind
#undef end_hierarchy
#undef end_simple_hierarchy
#undef escape_commas
