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

#ifndef BLACK_LOGIC_SUPPORT_HPP_
#define BLACK_LOGIC_SUPPORT_HPP_

namespace black::internal::new_api {
  
  class alphabet;

  //
  // This type enums the base hierarchy types like `formula`, `term`, etc...
  // Throughout the code, elements of this enum are used to identify the base 
  // hierarchy type when a precise syntax is not available, so the 
  // corresponding hierarchy class cannot be instantiated, e.g. `formula<S>` is 
  // not available because `S` is not known.
  //
  enum class hierarchy_type : uint8_t {
    #define declare_hierarchy(Base) Base,
    #include <black/new/internal/formula/hierarchy.hpp>
  };

  //
  // Enumeration similar to `hierarchy_type`, to identify specific storage
  // kinds.
  //
  enum class storage_type {

    #define declare_storage_kind(Base, Storage) Storage,
    #include <black/new/internal/formula/hierarchy.hpp>

  };

  //
  // This is the most important enumeration of the system. It lists all the
  // syntactic elements available in the whole hierarchy of types. The elements
  // listed here are all the types in the hierarchy which cannot be further
  // refined into subtypes. These corresponds to leaf storage kinds like
  // `boolean` and `proposition`, hierarchy elements like `conjunction` and
  // `disjunction`, and storage kinds without hierarchy elements such as `atom`.
  //
  enum class syntax_element : uint8_t {
    no_type,

    #define declare_leaf_storage_kind(Base, Storage) Storage,
    #define has_no_hierarchy_elements(Base, Storage) Storage,
    #define declare_hierarchy_element(Base, Storage, Element) Element,

    #include <black/new/internal/formula/hierarchy.hpp>
  };

  //
  // Here we start to define things related to the definition of the syntax of
  // logical fragments. The first thing is a plain compile-time list of values
  // of type `syntax_element`.
  //
  template<syntax_element ...Elements>
  struct type_list { };

  //
  // `is_type_list<T>` is a boolean trait that checks whether `T` is a
  // `type_list`.
  //
  template<typename T>
  struct is_type_list : std::false_type { };

  template<syntax_element ...Elements>
  struct is_type_list<type_list<Elements...>> : std::true_type { };

  template<typename T>
  constexpr bool is_type_list_v = is_type_list<T>::value;

  //
  // `TypeList` concept associated with the `is_type_list` trait.
  //
  template<typename T>
  concept TypeList = is_type_list_v<T>;

  //
  // The `AcceptsType` concept models types used to define, either a
  // compile-time or at runtime, whether a given `syntax_element` is allowed in
  // a given fragment or by a given storage kind. These types define a
  // `doesit()` member function that returns `true` if the syntax element is
  // accepted.
  //
  template<typename T>
  concept AcceptsType = requires(syntax_element e) {
    { T::doesit(e) } -> std::convertible_to<bool>;
  };

  //
  // `FragmentType` models types used as pseudo-enum types for enumerating
  // hierarchy elements of storage kinds, e.g. `unary<LTL>::type`. We can only
  // just model the fact that the type holds a value of type `syntax_element`.
  //
  template<typename T>
  concept FragmentType = requires(T t) {
    { t.type() } -> std::convertible_to<syntax_element>;
  };
  
  //
  // Dummy `AcceptsType` instance used in the following concept.
  //
  struct false_accepts_type {
    static constexpr bool doesit(syntax_element) { return false; }
  };

  //
  // Concept modeling the types representing logical fragments. These types
  // provide two member types:
  // 1. a `type_list` called `list`, providing the list of `syntax_element`s
  //    allowed in this fragment.
  // 2. a template accepting an `AcceptsType` type that, when instantiated,will
  //    give a `FragmentType`. This will be used as the pseudo-enum enumerating
  //    the allowed sub elements of storage kinds, such as `unary<LTL>::type`.
  //
  template<typename T>
  concept Fragment = requires {
    requires TypeList<typename T::list>;
    requires FragmentType<typename T::template type<false_accepts_type>>;
  };

  //
  // This concept models the most important types of the system, i.e. hierarchy
  // bases like `formula`, `term`, etc... and all the storage kinds such as
  // `unary`, `binary` and hierarchy elements such as `conjunction`. There are
  // many member types and constexpr members provided, used throughout the
  // system.
  // 1. `T::syntax` is the allowed `Fragment`;
  // 2. `T::accepts_type` tells what syntax_elements are accepted by the type.
  //    Note that for bases this is just the same information provided by
  //    `T::syntax`, but for storage kinds it is different. (i.e.
  //    `unary<LTL>::accepts_type` only accepts unary operators of LTL.
  // 3. `T::type` is just `T::syntax::type` instantiated with `T::accepts_type`.
  // 4. `T::syntax_elements` is used for matching. It is a `type_list` of
  //    elements from `T::syntax::list` filtered by `T::accepts_type`.
  // 5. `T::id_type` is the opaque type returned by `unique_id()` functions.
  // 6. `T::hierarchy` is the corresponding `hierarchy_type` 
  //
  // We also model the existence of `unique_id()`. Most other member functions
  // are templates and there is no way to check for their existence.
  //
  template<typename T>
  concept Hierarchy = requires(T t) {
    requires Fragment<typename T::syntax>;
    requires AcceptsType<typename T::accepts_type>;
    requires FragmentType<typename T::type>;
    requires TypeList<typename T::syntax_elements>;
    typename T::id_type;
    { T::hierarchy } -> std::convertible_to<hierarchy_type>;
    { t.unique_id() } -> std::convertible_to<typename T::id_type>;
  };

  //
  // This concept refines Hierarchy for storage kinds.
  //
  template<typename T>
  concept StorageKind = Hierarchy<T> && requires(T t) {
    { T::storage } -> std::convertible_to<storage_type>;
  };

  //
  // This concept refines StorageKind for hierarchy elements.
  //
  template<typename T>
  concept HierarchyElement = StorageKind<T> && requires(T t) {
    { T::element } -> std::convertible_to<syntax_element>;
  };

}

#endif // BLACK_LOGIC_SUPPORT_HPP_