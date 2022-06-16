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

#include <black/support/assert.hpp>
#include <black/support/hash.hpp>

#include <functional>
#include <numeric>
#include <ranges>

//
// This file contains all the declarations that do not depend on including the
// hierarchy definition file, i.e. everything that does not need the
// preprocessor. To understand what is happening here, please be sure to know
// how the logic API is used, and then read the comments in `hierarchy.hpp`
// first.
//

namespace black::internal::logic {
  
  //
  // `alphabet` is the main factory class for all the formulas. See its
  // definition for more details.
  //
  class alphabet;
  class alphabet_base;
  
  //
  // This type enums the base hierarchy types like `formula`, `term`, etc...
  // Throughout the code, elements of this enum are used to identify the base 
  // hierarchy type when a precise syntax is not available, so the 
  // corresponding hierarchy class cannot be instantiated, e.g. `formula<S>` is 
  // not available because `S` is not known.
  //
  enum class hierarchy_type : uint8_t;

  //
  // Enumeration similar to `hierarchy_type`, to identify specific storage
  // kinds.
  //
  enum class storage_type : uint8_t;

  //
  // This is the most important enumeration of the system. It lists all the
  // syntactic elements available in the whole hierarchy of types. The elements
  // listed here are all the types in the hierarchy which cannot be further
  // refined into subtypes. These corresponds to leaf storage kinds like
  // `boolean` and `proposition`, hierarchy elements like `conjunction` and
  // `disjunction`, and storage kinds without hierarchy elements such as `atom`.
  //
  enum class syntax_element : uint8_t;

  //
  // A trait to get the base hierarchy_type of a given storage_type. This is
  // only forward-declared here, and specialized later by preprocessing the
  // definitions file.
  //
  template<storage_type Storage>
  struct hierarchy_of_storage;

  template<storage_type Storage>
  inline constexpr auto hierarchy_of_storage_v = 
    hierarchy_of_storage<Storage>::value;
  
  //
  // Same thing as above, but to get the `storage_type` of a `syntax_element`
  //
  template<syntax_element Element>
  struct storage_of_element;

  template<syntax_element Element>
  inline constexpr auto storage_of_element_v = 
    storage_of_element<Element>::value;
  
  //
  // Last, we get the `syntax_element` of a leaf `storage_type`, or of one that
  // has no hierarchy elements. Note that this is a partial meta-function, since
  // non-leaf storage kinds with hierarchy elements (e.g. `unary`) are not
  // associated with a single `syntax_element`.
  //
  template<storage_type Storage>
  struct element_of_storage;

  template<storage_type Storage>
  inline constexpr auto element_of_storage_v = 
    element_of_storage<Storage>::value;

  //
  // Here we start to define things related to the definition of the syntax of
  // logical fragments. The first thing is a plain compile-time list of values
  // of type `syntax_element`.
  //
  template<syntax_element ...Elements>
  struct syntax_list { };

  //
  // `is_syntax_list<T>` is a boolean trait that checks whether `T` is a
  // `syntax_list`.
  //
  template<typename T>
  struct is_syntax_list : std::false_type { };

  template<syntax_element ...Elements>
  struct is_syntax_list<syntax_list<Elements...>> : std::true_type { };

  template<typename T>
  inline constexpr bool is_syntax_list_v = is_syntax_list<T>::value;

  //
  // Trait to extract the first element from a `syntax_list`
  //
  template<typename List>
  struct syntax_list_head;

  template<syntax_element Element, syntax_element ...Elements>
  struct syntax_list_head<syntax_list<Element, Elements...>> {
    static constexpr auto value = Element;
  };

  template<typename List>
  inline constexpr auto syntax_list_head_v = syntax_list_head<List>::value;

  //
  // Trait to extract the tail of a `syntax_list`
  //
  template<typename List>
  struct syntax_list_tail;

  template<syntax_element Element, syntax_element ...Elements>
  struct syntax_list_tail<syntax_list<Element, Elements...>> {
    using type = syntax_list<Elements...>;
  };

  template<typename List>
  constexpr auto syntax_list_tail_t = syntax_list_tail<List>::value;

  //
  // Trait to tell the length of a `syntax_list`
  //
  template<typename List>
  struct syntax_list_length;

  template<syntax_element ...Elements>
  struct syntax_list_length<syntax_list<Elements...>>
    : std::integral_constant<size_t, sizeof...(Elements)> { };

  template<typename List>
  inline constexpr auto syntax_list_length_v = syntax_list_length<List>::value;

  //
  // Trait to concatenate two syntax lists
  //
  template<typename T, typename U>
  struct syntax_list_concat;

  template<syntax_element ...E1, syntax_element ...E2>
  struct syntax_list_concat<syntax_list<E1...>, syntax_list<E2...>> {
    using type = syntax_list<E1..., E2...>;
  };

  template<typename T, typename U>
  using syntax_list_concat_t = typename syntax_list_concat<T,U>::type;

  //
  // Trait to remove duplicates from a `syntax_list`. This is useful later when
  // merging two fragments to obtain a combined fragment.
  //
  template <typename T, typename List>
  struct syntax_list_unique_ { 
    using type = T;
  };

  template <syntax_element... E1, syntax_element E, syntax_element... E2>
  struct syntax_list_unique_<syntax_list<E1...>, syntax_list<E, E2...>>
    : std::conditional_t<
        ((E == E1) || ...),
        syntax_list_unique_<syntax_list<E1...>, syntax_list<E2...>>,
        syntax_list_unique_<syntax_list<E1..., E>, syntax_list<E2...>>
    > { };

  template<typename List>
  struct syntax_list_unique : syntax_list_unique_<syntax_list<>, List> { };

  template <typename List>
  using syntax_list_unique_t = typename syntax_list_unique<List>::type;

  //
  // Trait to check whether a `syntax_list` contains a given `syntax_element`.
  //
  template<typename List, syntax_element Element>
  struct syntax_list_contains : std::false_type { };

  template<syntax_element ...Elements, syntax_element Element>
  struct syntax_list_contains<syntax_list<Element, Elements...>, Element> 
    : std::true_type { };

  template<
    syntax_element ...Elements, syntax_element Element1, syntax_element Element2
  >
  struct syntax_list_contains<syntax_list<Element1, Elements...>, Element2> 
    : syntax_list_contains<syntax_list<Elements...>, Element2> { };

  template<typename List, syntax_element Element>
  inline constexpr bool syntax_list_contains_v = 
    syntax_list_contains<List, Element>::value;

  //
  // Trait to check whether a syntax list in included in another
  //
  template<typename List, typename SubList>
  struct syntax_list_includes;

  template<typename List, syntax_element ...Elements>
  struct syntax_list_includes<List, syntax_list<Elements...>> 
    : std::bool_constant<(syntax_list_contains_v<List, Elements> && ...)> { };

  template<typename List, typename Sublist>
  inline constexpr bool syntax_list_includes_v = 
    syntax_list_includes<List, Sublist>::value;

  //
  // The `syntax_predicate` concept models types used to define, either a
  // compile-time or at runtime, whether a given `syntax_element` is allowed in
  // a given fragment or by a given storage kind. These types define a
  // `doesit()` member function that returns `true` if the syntax element is
  // accepted. These are usually accessed by an `::accepts_type` member type in
  // hierarchy objects.
  //
  template<typename T>
  concept syntax_predicate = requires(syntax_element e) {
    { T::doesit(e) } -> std::convertible_to<bool>;
  };

  //
  // This is the standard way to create a syntax predicate from a set of
  // `syntax_element`s. Since this template will be used from code generated by
  // the preprocessor, we provide also a version with a first dummy parameter in
  // order to avoid to deal with trailing commas.
  //
  template<typename List>
  struct make_syntax_predicate_t;

  template<syntax_element ...Elements>
  struct make_syntax_predicate_t<syntax_list<Elements...>> { 
    static constexpr bool doesit(syntax_element e) {
      return ((e == Elements) || ...);
    }
  };

  template<typename List>
  struct make_syntax_predicate {
    using type = make_syntax_predicate_t<List>;
  };

  template<int dummy, syntax_element ...Elements>
  using make_syntax_predicate_cpp = 
    make_syntax_predicate<syntax_list<Elements...>>;

  //
  // These are the templates that will be specialized using the preprocessor for
  // each hierarchy and storage kind. Here we only forward-declare them.
  //
  template<hierarchy_type Hierarchy>
  struct hierarchy_syntax_predicate;
  
  template<hierarchy_type Hierarchy>
  using hierarchy_syntax_predicate_t = 
    typename hierarchy_syntax_predicate<Hierarchy>::type;

  template<storage_type Storage>
  struct storage_syntax_predicate;
  
  template<storage_type Storage>
  using storage_syntax_predicate_t = 
    typename storage_syntax_predicate<Storage>::type;

  //
  // Trait to filter a `syntax_list` based on a `syntax_predicate`
  //
  template<typename List, syntax_predicate AcceptsType>
  struct syntax_list_filter;

  template<typename List, syntax_predicate AcceptsType>
  using syntax_list_filter_t = 
    typename syntax_list_filter<List, AcceptsType>::type;

  template<syntax_predicate AcceptsType>
  struct syntax_list_filter<syntax_list<>, AcceptsType>
    : std::type_identity<syntax_list<>> { };

  template<
    syntax_predicate AcceptsType, 
    syntax_element Element, syntax_element ...Elements
  >
  struct syntax_list_filter<syntax_list<Element, Elements...>, AcceptsType>
    : std::conditional<
        AcceptsType::doesit(Element), 
        syntax_list_concat_t<
          syntax_list<Element>, 
          syntax_list_filter_t<syntax_list<Elements...>, AcceptsType>
        >,
        syntax_list_filter_t<syntax_list<Elements...>, AcceptsType>
      >
    { };

  //
  // Function to check *at runtime* if a syntax element is allowed by the
  // element list of a fragment.
  //
  template<syntax_element ...Elements>
  bool is_syntax_element_allowed(syntax_element e, syntax_list<Elements...>) {
    return ((e == Elements) || ...);
  }

  // this empty class is used as a base class of `fragment_type` later, to mark
  // fragment types in the following concept definition.
  struct fragment_type_marker_base { };

  //
  // `fragment_enum` models types used as pseudo-enum types for enumerating
  // hierarchy elements of storage kinds, e.g. `unary<LTL>::type`. We can only
  // just model the fact that the type holds a value of type `syntax_element`,
  // and that it derives from `fragment_type_marker_base`. It tries to mimic
  // being an actual enum class by being explicitly convertible to its underlyng
  // type.
  //
  template<typename T>
  concept fragment_enum = requires(T t) {
    requires std::derived_from<T, fragment_type_marker_base>;
    { syntax_element(t) };
    { uint8_t(t) };
  };

  //
  // Dummy `syntax_predicate` instance used in the following concept.
  //
  struct false_syntax_predicate {
    static constexpr bool doesit(syntax_element) { return false; }
  };

  //
  // Concept modeling the types representing logical fragments. These types
  // provide two member types:
  // 1. a `syntax_list` called `list`, providing the list of `syntax_element`s
  //    allowed in this fragment.
  // 2. a template accepting an `syntax_predicate` type that, when
  //    instantiated, will give a `fragment_enum`.
  //
  template<typename T>
  concept fragment = requires {
    requires is_syntax_list_v<typename T::list>;
    requires fragment_enum<
      typename T::template type<T, false_syntax_predicate>
    >;
  };

  //
  // The `fragment_type` class will provide the concrete instances of the
  // `fragment_enum` concept. For a given `syntax_predicate` `P` and a
  // `syntax_list` `S`, `fragment_type<P, S>` will have one constexpr member
  // named exactly `Element` for each syntax element from `S` allowed by `P`.
  // For example, `unary<LTL>::type` has members `unary<LTL>::type::negation`,
  // `unary<LTL>::type::always`, etc... The concrete names will be injected by
  // the preprocessor later. Here, we define the needed types.
  //
  // We first declare a struct whose only purpose is to encapsulate a statically
  // known `syntax_element`.
  template<syntax_element Element>
  struct type_value {
    static constexpr syntax_element value = Element;

    explicit operator syntax_element() const { return Element; }
  };

  //
  // Because `type_value` is supposed to be used in pattern matching expressions
  // against `fragment_type` objects (see below), we need to define the
  // Tuple-like interface. In this case, only `tuple_size` since there is no
  // field to unpack.
  //
  } namespace std {
    template<black::internal::logic::syntax_element Element>
    struct tuple_size<black::internal::logic::type_value<Element>> 
      : integral_constant<size_t, 0> { };
  } namespace black::internal::logic {

  //
  // Then, an incomplete template class that will be specialized by the
  // preprocessor with the concrete names of the pseudo-enum values, e.g. we
  // will have:
  //
  // template<> 
  // struct fragment_enum_element<syntax_element::conjunction> {
  //   static constexpr 
  //   type_value<syntax_element::conjunction> conjunction;
  // };
  template<syntax_element Element>
  struct fragment_enum_element;

  //
  // Then, a class that derives from all the `fragment_enum_element`s of a given
  // syntax list, used as a base class for `fragment_type`. It also derives from
  // a particular empty base class used in the definition of the `fragment_enum`
  // concept.
  //
  template<typename List>
  struct fragment_type_base;

  template<syntax_element ...Elements>
  struct fragment_type_base<syntax_list<Elements...>>
    : fragment_type_marker_base, fragment_enum_element<Elements>... { };

  //
  // Now we can define `fragment_type`, which derives from `fragment_type_base`
  // after filtering the list by the given predicate.
  //
  // The type itself is simple, and it only carries over the currently assigned
  // `syntax_element`. Publicly, it can be only constructed by
  // `type_value`s corresponding to syntax elements included in its
  // list. A private constructor constructs from `syntax_element` directly, and
  // is accessible only by the type `Owner` specified by the template parameter.
  //
  // Since this is not a real enum, it is not usable in common `switch()`
  // statements. However, we can expose the `match()` function, similar to the
  // one exposed by hierarchy types. Here, we only have to expose a `to<>()`
  // member function template for downcasting to `type_value`s. The
  // `match()` function is implemented later, after the machinery for the
  // pattern matching functionality.
  //
  template<typename Owner, typename List>
  class fragment_type : public fragment_type_base<List> 
  {
  public:
    using list = List;

    fragment_type() = delete;

    fragment_type(fragment_type const&) = default;
    fragment_type(fragment_type &&) = default;

    fragment_type &operator=(fragment_type const&) = default;
    fragment_type &operator=(fragment_type &&) = default;

    template<syntax_element Element>
      requires syntax_list_contains_v<list, Element>
    fragment_type(type_value<Element>) : _element{Element} { }

    template<typename O, typename L2>
      requires syntax_list_includes_v<list, L2>
    fragment_type(fragment_type<O, L2> const&t) 
      : _element{syntax_element{t}} { }

    template<typename O, typename L2>
    bool operator==(fragment_type<O, L2> const& t) const {
      return _element == syntax_element{t};
    }
    
    template<typename T>
    std::optional<T> to() const {
      if(_element == T::value)
        return {T{}};
      else
        return {};
    }

    template<typename T>
    bool is() const { 
      return to<T>().has_value();
    }

    template<typename ...Handlers>
    auto match(Handlers ...) const;

    operator syntax_element() const { return _element; }
    explicit operator uint8_t() const { return uint8_t(_element); }

  private:
    friend Owner;

    explicit fragment_type(syntax_element e) : _element{e} { 
      black_assert(is_syntax_element_allowed(e, list{}));
    }

  private:
    syntax_element _element;
  };

  template<typename O, typename L2, syntax_element E>
  bool operator==(fragment_type<O, L2> const& t, type_value<E>) {
    return syntax_element{t} == E;
  }
  
  template<typename O, typename L2, syntax_element E>
  bool operator==(type_value<E>, fragment_type<O, L2> const& t) {
    return E == syntax_element{t};
  }

  //
  // Now we can define actual fragments. To make a fragment made of some given
  // `syntax_element`s, just call `make_fragment_t<Elements...>`
  //
  template<syntax_element ...Elements>
  struct make_fragment_t {
    using list = syntax_list_unique_t<syntax_list<Elements...>>;
    
    template<typename Owner, syntax_predicate AcceptsType>
    using type = fragment_type<Owner, syntax_list_filter_t<list, AcceptsType>>;
  };

  template<syntax_element ...Elements>
  struct make_fragment {
    using type = make_fragment_t<Elements...>;
  };

  //
  // Helper trait to use `make_fragment` in the preprocessed code and handle
  // trailing commas properly.
  //
  template<int Dummy, syntax_element ...Elements>
  struct make_fragment_cpp : make_fragment<Elements...> { };

  template<int Dummy, syntax_element ...Elements>
  using make_fragment_cpp_t = make_fragment_t<Elements...>;

  //
  // Trait to tell whether a fragment is subsumed by another. This trait is used
  // in every place where a conversion between different syntaxes is requested,
  // e.g. from formula<Boolean> to formula<LTL>.
  //
  template<fragment Fragment, fragment Allowed>
  struct is_subfragment_of : syntax_list_includes<
    typename Allowed::list,
    typename Fragment::list
  > { };
  
  template<fragment Fragment, fragment Allowed>
  inline constexpr bool is_subfragment_of_v = 
    is_subfragment_of<Fragment, Allowed>::value;

  //
  // Fragments can also be created by combining other fragments. This is a bit
  // complex because we want to avoid creating bigger and bigger types (in terms
  // of instantiation depth) when we compose `make_combined_fragment_t` multiple
  // times, and we also want to short-circuit common cases like combining a
  // fragment with a proper subfragment of itself.
  //
  // For example, `make_combined_fragment_t<Boolean, LTL>` should be exactly
  // `LTL`, not some more complex type leading to an equivalent fragment.
  //
  // So first we declare the type doing the actual combination of two fragments.
  //
  template<fragment Fragment1, fragment Fragment2>
  struct make_combined_fragment_impl_t {
    using list = syntax_list_unique_t<
      syntax_list_concat_t<typename Fragment1::list, typename Fragment2::list>
    >;

    template<typename Owner, syntax_predicate AcceptsType>
    using type = fragment_type<Owner, syntax_list_filter_t<list, AcceptsType>>;
  };

  //
  // Now we implement a trait on top of that to simplify common scenarios.
  //
  template<fragment Fragment1, fragment Fragment2>
  struct make_combined_fragment_simplified {
    using type = make_combined_fragment_impl_t<Fragment1, Fragment2>;
  };

  template<fragment Fragment1, fragment Fragment2>
    requires is_subfragment_of_v<Fragment1, Fragment2>
  struct make_combined_fragment_simplified<Fragment1, Fragment2> { 
    using type = Fragment2;
  };
  
  template<fragment Fragment1, fragment Fragment2>
    requires (!is_subfragment_of_v<Fragment1, Fragment2> &&
              is_subfragment_of_v<Fragment2, Fragment1>)
  struct make_combined_fragment_simplified<Fragment1, Fragment2> { 
    using type = Fragment1;
  };

  //
  // And then the final vararg version that puts everything together
  //
  template<fragment ...Fragments>
  struct make_combined_fragment;

  template<>
  struct make_combined_fragment<> : make_fragment<> { };

  template<fragment Fragment, fragment ...Fragments>
  struct make_combined_fragment<Fragment, Fragments...> 
    : make_combined_fragment_simplified<
        Fragment, typename make_combined_fragment<Fragments...>::type
      > { };

  template<fragment ...Fragments>
  using make_combined_fragment_t = 
    typename make_combined_fragment<Fragments...>::type;

  //
  // This type, defined later, gives us the fragment made of all the
  // `syntax_element`s of all the hierarchies. It is used as a default argument
  // for the fragment template parameter of hierarchy types.
  //
  struct universal_fragment_t;

  //
  // We start to prepare for the declaration of actual hierarchy types such as
  // `formula`, `unary`, `conjunction` etc... Each of these types is a
  // lightweight wrapper over two pointers, one to the alphabet, and one to the
  // actual object that they are a handle for. The type of such object is
  // internal and not intended to be accessed by the user.
  //
  // The following is the base class of all of such types. There is one base
  // class for each hierarchy type, so this is a template.
  //
  template<hierarchy_type Hierarchy>
  struct hierarchy_node {
    syntax_element type;

    bool operator==(hierarchy_node const&) const = default;
  };

  //
  // This is an opaque type used as a return type for the `unique_id()` member
  // function of the hierarchy types. Useful to compare for identity, hash, or
  // print a unique identifier of hierarchy objects.
  //
  template<hierarchy_type H>
  struct hierarchy_unique_id { 
    enum class type : uintptr_t { };
  };

  template<hierarchy_type H>
  using hierarchy_unique_id_t = typename hierarchy_unique_id<H>::type;

  //
  // This concept models the most important types of the system, i.e. hierarchy
  // bases like `formula`, `term`, etc... and all the storage kinds such as
  // `unary`, `binary` and hierarchy elements such as `conjunction`. There are
  // many member types and constexpr members provided, used throughout the
  // system.
  // 1. `T::syntax` is the allowed `fragment`;
  // 2. `T::accepts_type` tells what syntax_elements are accepted by the type.
  //    This is the information provided by `T::syntax`, but filtered for the
  //    specific hierarchy at hand. For example, `formula<FO>::accepts_type`
  //    only acceptst the elements from the FO fragment that are formulas (e.g.
  //    no terms). As another example, `unary<LTL>::accepts_type` only accepts
  //    unary operators of LTL.
  // 3. `T::type` is just `T::syntax::type` instantiated with `T::accepts_type`.
  // 4. `T::hierarchy` is the corresponding `hierarchy_type` 
  //
  // We also model the existence of some member functions, while others are
  // templates and there is no way to check for their existence.
  //
  template<typename T>
  concept hierarchy = requires(T t) {
    requires fragment<typename T::syntax>;
    requires syntax_predicate<typename T::accepts_type>;
    requires fragment_enum<typename T::type>;
    { T::hierarchy } -> std::convertible_to<hierarchy_type>;

    { t.unique_id() } -> 
      std::convertible_to<hierarchy_unique_id_t<T::hierarchy>>;
    { t.sigma() } -> std::convertible_to<alphabet *>;
    { t.hash() } -> std::convertible_to<size_t>;
    { t.node_type() } -> std::convertible_to<typename T::type>;

    // we should constrain the return type of `node()`, but then checking this
    // concept would force the instantiation of `hierarchy_node<>`, which is
    // only defined much later in the preprocessed definition file
    { t.node() };
    
    // missing members:
    // to<H>() function to convert to another compatible hierarchy type H
    // is<H>() function to tell if it can be converted to H
    // match(...) function for pattern matching
  };

  //
  // This trait type provides information computed from the given `hierarchy`
  // type.
  // 1. `syntax` and `accepts_type` and `type` are just exported from the
  //    hierarchy type.
  // 2. `accepted_elements` is the syntax list from `syntax` filtered by
  //    `accepts_type`.
  // 3. `unique_id_t` is the unique type id from `hierarchy_unique_id_t`
  //
  template<hierarchy H>
  struct hierarchy_traits {
    using syntax = typename H::syntax;
    using accepts_type = typename H::accepts_type;
    using type = typename H::type;
    using accepted_elements =
      syntax_list_filter_t<typename syntax::list, typename H::accepts_type>;
    using unique_id_t = hierarchy_unique_id_t<H::hierarchy>;
  };

  //
  // hierarchy types are hashable
  //
  } namespace std {
    template<black::internal::logic::hierarchy H>
    struct hash<H> {
      size_t operator()(H h) const {
        return h.hash();
      }
    };
  } namespace black::internal::logic {

  //
  // Similarly to hierarchy_node<>, we have storage_node<> as a concrete node
  // type for storage kinds. Its members however must be personalized for each
  // storage kind from the definitions file, so we declare here a trait that
  // will return the concrete type for each element, and will be defined later
  // in the preprocessed part of the code. 
  //
  template<storage_type Storage>
  struct storage_data { };

  template<storage_type Storage>
  using storage_data_t = typename storage_data<Storage>::type;

  //
  // All the specializations of `storage_data` return an instance of the
  // following class template, which simply wraps a tuple of elements. We wrap
  // the tuple because this type will have to be Hashable, but we do not want to
  // define specializations of types of the standard library in a library's
  // header files. So we define the wrapper and provide the instances of
  // `tuple_size`, `tuple_element` and `get<>` for easy access later.
  template<typename ...Types>
  struct make_storage_data_t {
    using tuple_type = std::tuple<Types...>;

    template<typename ...Args>
    make_storage_data_t(Args&& ...args)  
      : values{std::forward<Args>(args)...} { }

    tuple_type values;
  };

  //
  // The trait has a dummy `int` template parameter in order to ease the
  // handling of trailing commas in the macros. We also provide a
  // preprocessor-friendly version to handle trailing commas properly.
  //
  template<typename ...Types>
  struct make_storage_data {
    using type = make_storage_data_t<Types...>;
  };

  template<int Dummy, typename ...Types>
  struct make_storage_data_cpp : make_storage_data<Types...> { };

  //
  // Here we define the traits for the tuple-like access to `storage_data_t`
  //
  } namespace std {

    template<typename ...Types>
    struct tuple_size<
      black::internal::logic::make_storage_data_t<Types...>
    > : std::integral_constant<size_t, sizeof...(Types)> { };

    template<size_t I, typename ...Types>
    struct tuple_element<I, 
      black::internal::logic::make_storage_data_t<Types...>
    > : tuple_element<I, tuple<Types...>> { };

  } namespace black::internal::logic {

  //
  // And the function get<> to access the elements
  //
  template<size_t I, typename ...Types>
  std::tuple_element_t<I, make_storage_data_t<Types...>> const&
  get(make_storage_data_t<Types...> const& data) {
    return std::get<I>(data.values);
  }
  
  //
  // To make `make_storage_data_t` hashable, we provide a specialization of
  // std::hash<>
  //
  } namespace std {

    template<typename ...Types>
    struct hash<black::internal::logic::make_storage_data_t<Types...>> 
    {
      size_t operator()(
        black::internal::logic::make_storage_data_t<Types...> const& data
      ) {
        using namespace black::internal::logic;
        using namespace black::internal;

        size_t h = 0;
        std::apply([&]<typename ...Ts>(Ts const& ...values) {
          ((h = hash_combine(h, std::hash<Ts>{}(values))), ...);
        }, data.values);

        return h;
      }
    };

  } namespace black::internal::logic {

  //
  // Last thing to make `storage_data_t` hashable is a proper `operator==`
  //
  // However, the standard `operator==` of tuples is not enough. Some hierarchy
  // types, which may end up as types of fields of `make_storage_data_t` (think
  // of the `var` field of `quantifier` which is of type `variable`), might have
  // a custom `operator==` (this happens for `term`s). However, that is defined
  // much later, because we do not have concrete hierarchy types here yet. So we
  // define this helper function that compares everything with the standard
  // `operator==` except for two `hierarchy`s that are compared by
  // `unique_id()`.
  //
  template<hierarchy T1, hierarchy T2>
  bool are_equal(T1 t1, T2 t2) {
    return t1.unique_id() == t2.unique_id();
  }

  template<typename T1, typename T2>
  bool are_equal(T1 t1, T2 t2) {
    return t1 == t2;
  }

  //
  // Now the `operator==` for `make_storage_data_t`. Since we have to zip the
  // two tuples we need an auxiliary function to unpack an index sequence.
  //
  template<size_t ...Idx, typename ...Types>
  bool storage_data_t_cmp_aux(
    make_storage_data_t<Types...> const& k1,
    make_storage_data_t<Types...> const& k2,
    std::index_sequence<Idx...>
  ) {
    return 
      (are_equal(std::get<Idx>(k1.values), std::get<Idx>(k2.values)) && ...);
  }

  template<typename ...Types>
  bool operator==(
    make_storage_data_t<Types...> const& k1,
    make_storage_data_t<Types...> const& k2
  ) {
    return storage_data_t_cmp_aux(k1, k2, std::index_sequence_for<Types...>{});
  }

  //
  // Now we can finally declare the actual `storage_node` type, which inherits
  // from `hierarchy_node` and wraps the corresponding `storage_data_t`.
  //
  template<storage_type Storage>
  struct storage_node : hierarchy_node<hierarchy_of_storage_v<Storage>>
  { 
    template<typename ...Args>
    storage_node(syntax_element element, Args&& ...args)
      : hierarchy_node<hierarchy_of_storage_v<Storage>>{element}, 
        data{std::forward<Args>(args)...} { }

    storage_data_t<Storage> data;

    bool operator==(storage_node const&) const = default;
  };

  //
  // We will use storage nodes themselves as a key in the alphabet's hash tables
  // to retrieve the actual uniqued nodes. Thus we need `storage_node` to be
  // hashable. This is based on the `std::hash` instance of `storage_data_t`,
  // defined above.
  //
  } namespace std {
    template<black::internal::logic::storage_type Storage>
    struct hash<black::internal::logic::storage_node<Storage>> {
      size_t operator()(
        black::internal::logic::storage_node<Storage> const&n
      ) const {
        using namespace black::internal::logic;
        using namespace black::internal;
        
        size_t type_hash = std::hash<syntax_element>{}(n.type);
        size_t data_hash = std::hash<storage_data_t<Storage>>{}(n.data);
        return hash_combine(type_hash, data_hash);
      }
    };
  } namespace black::internal::logic {

  //
  // This concept refines `hierarchy` for storage kinds.
  //
  template<typename T>
  concept storage_kind = hierarchy<T> && requires(T t) {
    { T::storage } -> std::convertible_to<storage_type>;
  };

  //
  // This concept refines `storage_kind` for hierarchy elements.
  //
  template<typename T>
  concept hierarchy_element = storage_kind<T> && requires(T t) {
    { T::element } -> std::convertible_to<syntax_element>;
  };

  //
  // We also forward declare an empty CRTP class that can be specialized by user
  // code to provide custom members to `hierarchy_base` defined below.
  //
  template<hierarchy_type Hierarchy, typename Derived>
  struct hierarchy_custom_members { };

  //
  // We can now declare the base class for all the hierarchy types. This class
  // provides all the necessary infrastructure to obtain a complete hierarchy
  // type. Concrete hierarchy types are obtained by preprocessing the hierarchy
  // definition file by just deriving from this base class and re-exporting the
  // constructors.
  //
  template<hierarchy_type Hierarchy, fragment Syntax>
  class hierarchy_base 
  {
    using node_t = hierarchy_node<Hierarchy>;
  public:
    // members required by the `hierarchy` concept
    using syntax = Syntax;
    using accepts_type = hierarchy_syntax_predicate_t<Hierarchy>;
    using type = typename Syntax::template type<hierarchy_base, accepts_type>;
    static constexpr auto hierarchy = Hierarchy;

    // hierarchy types are not default constructible but are
    // copy/move/constructible/assignable
    hierarchy_base() = delete;
    hierarchy_base(hierarchy_base const&) = default;
    hierarchy_base(hierarchy_base &&) = default;

    hierarchy_base &operator=(hierarchy_base const&) = default;
    hierarchy_base &operator=(hierarchy_base &&) = default;

    // this constructor is for internal use but has to be public (for now)
    hierarchy_base(alphabet_base *sigma, node_t const*node)
      : _sigma{sigma}, _node{node} { }

    // converting constructor from other hierarchy types
    // the conversion only happen for the same kind of hierarchy (e.g. formulas)
    // and only if the argument's syntax is a subfragment of our syntax
    template<::black::internal::logic::hierarchy H>
      requires (H::hierarchy == Hierarchy && 
                is_subfragment_of_v<typename H::syntax, syntax>)
    hierarchy_base(H h) : hierarchy_base{h.sigma(), h.node()} { }

    //
    // other member functions
    //
    template<::black::internal::logic::hierarchy H>
    std::optional<H> to() const {
      return H::from(*this);
    }

    template<::black::internal::logic::hierarchy H>
    bool is() const {
      return to<H>().has_value();
    }

    // implemented later
    template<typename ...Handlers>
    auto match(Handlers ...handlers) const;

    auto unique_id() const {
      return hierarchy_unique_id_t<hierarchy>{
        reinterpret_cast<uintptr_t>(_node)
      };
    }

    size_t hash() const {
      return std::hash<hierarchy_node<hierarchy> const*>{}(_node);
    }

    type node_type() const {
      return type{_node->type};
    }
    
    // we make `sigma()` a function template because `alphabet` is still
    // incomplete at this point, otherwise the static_cast would be invalid.
    template<typename A = alphabet>
    A *sigma() const { return static_cast<A *>(_sigma); }

    auto node() const { return _node; }

  private:
    alphabet_base *_sigma;
    node_t const*_node;
  };

  //
  // We can obtain the hierarchy type from the `hierarchy_type` value with the
  // following trait, which will be specialized in the preprocessing code.
  //
  template<fragment Syntax, hierarchy_type H>
  struct hierarchy_type_of;

  template<fragment Syntax, hierarchy_type H>
  using hierarchy_type_of_t = 
    typename hierarchy_type_of<Syntax, H>::type;

  //
  // In general, hierarchy types are equality comparable in a standard way, but
  // the definition file can opt out of it to customize the comparison. What
  // follows set up things to handle that.
  //
  // The first thing is a specializable trait to signal that a hierarchy type
  // has or not a standard equality comparison operator.
  //
  template<hierarchy_type H>
  struct hierarchy_has_standard_equality : std::true_type { };

  template<hierarchy_type H>
  inline constexpr bool hierarchy_has_standard_equality_v =
    hierarchy_has_standard_equality<H>::value;

  //
  // Here we define the standard comparison operators.
  //
  template<hierarchy H1, hierarchy H2>
    requires (H1::hierarchy == H2::hierarchy && 
             hierarchy_has_standard_equality_v<H1::hierarchy>)
  bool operator==(H1 h1, H2 h2) {
    return h1.unique_id() == h2.unique_id();
  }
 
  template<hierarchy H1, hierarchy H2>
    requires (H1::hierarchy == H2::hierarchy && 
             hierarchy_has_standard_equality_v<H1::hierarchy>)
  bool operator!=(H1 h1, H2 h2) {
    return h1.unique_id() != h2.unique_id();
  }

  //
  // This small trait tells us whether a storage kind has hierarchy elements.
  // It is specialized later by the preprocessed code.
  //
  template<storage_type Storage>
  struct storage_has_hierarchy_elements : std::true_type { };

  template<storage_type Storage>
  inline constexpr bool storage_has_hierarchy_elements_v =
    storage_has_hierarchy_elements<Storage>::value;

  //
  // This concept tells whether a given `storage_kind` is leaf or not. This can
  // be detected by looking at size of its fragment. If the fragment is a
  // singleton, we have a leaf.
  //
  template<typename S>
  concept leaf_storage_kind = storage_kind<S> &&
    (syntax_list_length_v<typename S::syntax::list> == 1);

  //
  // hierarchy types for storage kinds need to provide access to fields and
  // children. Here we declare two empty CRTP base classes that will be
  // specialized later by the preprocessed hierarchy definition file to provide
  // access to those members.
  //
  template<storage_type Storage, typename Derived>
  struct storage_fields_base { };

  template<storage_type Storage, fragment Syntax, typename Derived>
  struct storage_children_base { };

  //
  // Similar to `hierarchy_custom_members`, we declare a little empty CRTP class
  // to let anybody add custom members to `storage_base`.
  //
  template<storage_type Storage, typename Derived>
  struct storage_custom_members { };

  //
  // To define hierarchy types for storage kinds is more complex because they
  // have to call back to the alphabet to allocate the underlying nodes. Hence
  // we have to also consider the definition of the `alphabet` class. Thus, the
  // "allocating constructor" of storage types has to be defined in the
  // generated code. Nevertheless, here we can declare all the rest, and we can
  // inherit directly from hierarchy_base<> for all the common parts.
  //
  // Note that this base class is for non-leaf storage kinds with at least a
  // hierarchy element. Leaf storage kinds, or storage kinds with no hierarchy
  // elements, are more similar to hierarchy elements and thus inherit from
  // `hierarchy_element_base` declared below. Differently from `hierarchy_base`,
  // this is a CRTP base class.
  //
  template<storage_type Storage, fragment Syntax, typename Derived>
  class storage_base 
    : public hierarchy_base<hierarchy_of_storage_v<Storage>, Syntax>,
      public storage_fields_base<Storage, Derived>,
      public storage_children_base<Storage, Syntax, Derived>,
      public storage_custom_members<Storage, Derived>
  {
    using node_t = storage_node<Storage>;
    using base_t = hierarchy_base<hierarchy_of_storage_v<Storage>, Syntax>;
  public:
    // these members from the base have to be overriden and specialized
    using accepts_type = storage_syntax_predicate_t<Storage>;
    using type = typename Syntax::template type<storage_base, accepts_type>;
    static constexpr storage_type storage = Storage;

    storage_base() = default;
    storage_base(storage_base const&) = default;
    storage_base(storage_base &&) = default;

    storage_base &operator=(storage_base const&) = default;
    storage_base &operator=(storage_base &&) = default;

    // the wrapping constructor delegates to the base's one
    storage_base(alphabet_base *sigma, node_t const*node) 
      : base_t{sigma, node} { 
      black_assert(accepts_type::doesit(node->type));
    }

    // converting constructors from other storages of the same kind. 
    template<storage_kind S>
      requires (S::storage == storage && 
                is_subfragment_of_v<
                  typename S::syntax, typename base_t::syntax
                >)
    storage_base(S s) : storage_base{s.sigma(), s.node()} { }

    // we override node() to return a more specific node type
    node_t const*node() const { 
      return static_cast<node_t const*>(base_t::node()); 
    }

    // we override match() to be more specific in the list of possible cases. 
    // Implemented later.
    template<typename ...Handlers>
    auto match(Handlers ...handlers) const;
    
    // we override `type()` from `hierarchy_base` to use our `type`.
    type node_type() const {
      return type{node()->type};
    }

    // this static member function does the job of the `to<>` and `is<>` members
    // of `hierarchy_base`. The conversion takes place if the fragments agree,
    // and the actual `syntax_element` of the node at runtime is the correct
    // one.
    //
    // The fragment of the original type (`F`) must be a subfragment of ours.
    // But this does not holds for leaves, which always have a singleton
    // fragment. So for example, if `f` is a `formula<LTL>` which concretely is
    // a `negation<LTL>`, `f.to<negation<Boolean>>()` is `false` because `LTL`
    // is not a subfragment of `Boolean`. However, if `f` is a `proposition`
    // (whose fragment always contains only `syntax_element::proposition`), we
    // cannot make `f.to<proposition>()` fail for the same reason. On the other
    // hand, `proposition` does not have any children (it's a leaf), so we do
    // not risk downcasting the fragment of children.
    //
    // The dummy template parameter `D` is always used with its default argument
    // equal to `Derived`, and is needed to access to Derived::syntax, which
    // otherwise would not be available because `Derived` is not complete at
    // this point.
    template<hierarchy F, typename D = Derived>
      requires (F::hierarchy == D::hierarchy)
    static std::optional<Derived> from(F f) {
      if constexpr(
        !leaf_storage_kind<Derived> &&
        !is_subfragment_of_v<typename F::syntax, typename base_t::syntax>
      ) return {};

      // note the subtlety here: we have to use Derived::accepts_type to get the
      // right set of accepted elements.
      using derived_accepts_type = typename Derived::accepts_type;
      if(!derived_accepts_type::doesit(f.node()->type))
        return {};

      auto obj = static_cast<node_t const *>(f.node());
      return std::optional<Derived>{Derived{f.sigma(), obj}};
    }
  };

  //
  // Similarly to `hierarchy_type_of`, we can obtain the concrete storage type
  // of a `storage_type` with this trait, specialized later. Note that the
  // `Syntax` parameter will be ignored if a leaf storage kind is requested.
  //
  template<fragment Syntax, storage_type H>
  struct storage_type_of;

  template<fragment Syntax, storage_type H>
  using storage_type_of_t = 
    typename storage_type_of<Syntax, H>::type;

  //
  // Concrete storage types without hierarchy elements (e.g. `atom<>`) have
  // deduction guides that help avoid to specify the fragment each time. The
  // deduction guide has to sum up all the fragments of the children into a
  // combined fragment. We take the fragments from the arguments, if available.
  // We can get the fragment from an argument of hierarcy type or from a vector
  // of hierarchy types.
  //
  // The following is a concept that tells us whether an argument is one of
  // those two cases.
  //
  template<typename T>
  concept can_get_fragment = hierarchy<T> || 
    (std::ranges::range<T> && hierarchy<std::ranges::range_value_t<T>>);

  //
  // This trait actually extracts the fragment from the argument.
  //
  template<typename Arg, typename = void>
  struct get_fragment_from_arg { };

  template<hierarchy Arg>
  struct get_fragment_from_arg<Arg> { 
    using type = typename Arg::syntax;
  };
  
  template<std::ranges::range Arg>
    requires hierarchy<std::ranges::range_value_t<Arg>>
  struct get_fragment_from_arg<Arg> {
    using type = typename std::ranges::range_value_t<Arg>::syntax;
  };

  template<typename Arg>
  using get_fragment_from_arg_t = typename get_fragment_from_arg<Arg>::type;
  
  //
  // Then, this trait combines the found fragments into one.
  //
  template<typename ...Args>
  struct combined_fragment_from_args;

  template<typename ...Args>
  using combined_fragment_from_args_t = 
    typename combined_fragment_from_args<Args...>::type;

  template<can_get_fragment Arg>
  struct combined_fragment_from_args<Arg> : get_fragment_from_arg<Arg> { };

  template<can_get_fragment Arg, typename ...Args>
  struct combined_fragment_from_args<Arg, Args...> 
    : make_combined_fragment<
        get_fragment_from_arg_t<Arg>,
        combined_fragment_from_args_t<Args...>
      > { };
  
  template<typename Arg, typename ...Args>
  struct combined_fragment_from_args<Arg, Args...>
    : combined_fragment_from_args<Args...> { };

  //
  // We sum up everything here, ready for use in the deduction guide
  //
  template<syntax_element Element, typename ...Args>
  struct deduce_fragment_for_storage :
    make_combined_fragment< \
      make_fragment_t<Element>, \
      combined_fragment_from_args_t<Args...> \
    > { };

  template<syntax_element Element, typename ...Args>
  using deduce_fragment_for_storage_t = 
    typename deduce_fragment_for_storage<Element, Args...>::type;

  //
  // Similar to `hierarchy_custom_members`, we declare a little empty CRTP class
  // to let anybody add custom members to `storage_base`.
  //
  template<syntax_element Element, typename Derived>
  struct hierarchy_element_custom_members { };

  //
  // The following is the last of the three kinds of hierarchy types. Hierarchy
  // elements are the leaves of the hierarchy tree. They are associated to a
  // single `syntax_element` with no more uncertainty. This is a CRTP class as
  // well.
  //
  template<syntax_element Element, fragment Syntax, typename Derived>
  class hierarchy_element_base
    : public storage_base<storage_of_element_v<Element>, Syntax, Derived>,
      public hierarchy_element_custom_members<Element, Derived>
  {
    using node_t = storage_node<storage_of_element_v<Element>>;
    using base_t = 
      storage_base<storage_of_element_v<Element>, Syntax, Derived>;

  public:
    // these members from the base have to be overriden and specialized
    using accepts_type = make_syntax_predicate_t<syntax_list<Element>>;
    using type = typename base_t::syntax::template type<
      hierarchy_element_base, accepts_type
    >;
    static constexpr syntax_element element = Element;

    hierarchy_element_base() = delete;
    hierarchy_element_base(hierarchy_element_base const&) = default;
    hierarchy_element_base(hierarchy_element_base &&) = default;

    hierarchy_element_base &operator=(hierarchy_element_base const&) = default;
    hierarchy_element_base &operator=(hierarchy_element_base &&) = default;

    // the wrapping constructor delegates to the base's one
    hierarchy_element_base(alphabet_base *sigma, node_t const*node) 
      : base_t{sigma, node} {
      black_assert(accepts_type::doesit(node->type));
    }

    // converting constructor only from other equal elements with compatible
    // syntax.
    template<hierarchy_element E>
      requires (E::element == element &&
                is_subfragment_of_v<
                  typename E::syntax, typename base_t::syntax
                >)
    hierarchy_element_base(E e) 
      : hierarchy_element_base{e.sigma(), e.node()} { }

    // match() is useless on hierarchy elements but we keep it for generic
    // code. Implemented later.
    template<typename ...Handlers>
    auto match(Handlers ...handlers) const;

    // we override `type()` from `hierarchy_base` to use our `type`.
    type node_type() const {
      return type{this->node()->type};
    }
  };

  //
  // Sometimes it is useful (e.g. in tests), to compare two hierarchy types for
  // equivalence. Exact equality of types can be difficult to achieve because of
  // different fragments being equivalent (e.g. different calls to
  // `make_combined_fragment`). Hence, here we define a trait
  // `are_same_hierarchy_types` to smartly compare for equivalence two hierarchy
  // types.
  //
  template<hierarchy T, hierarchy U>
  struct are_same_hierarchy_types : 
    std::conjunction<std::is_convertible<T, U>, std::is_convertible<U, T>> { };

  template<hierarchy T, hierarchy U>
  inline constexpr bool are_same_hierarchy_types_v = 
    are_same_hierarchy_types<T,U>::value;

  //
  // Similarly to `hierarchy_type_of` and `storage_type_of`, we can obtain the
  // concrete type of a `syntax_element` with this trait, specialized later.
  // Note that the `Syntax` parameter will be ignored if a leaf hierarchy
  // element is requested. Note also that the returned type might not be a
  // `hierarchy_element_base` but only a `storage_base`, in the case of leaf
  // storage kinds or storage kinds with no hierarchy element.
  //
  template<fragment Syntax, syntax_element E>
  struct element_type_of;

  template<fragment Syntax, syntax_element E>
  using element_type_of_t = 
    typename element_type_of<Syntax, E>::type;

  //
  // Now let's talk about how storage kinds are allocated. The actual allocation
  // happens inside the alphabet class, which is autogenerated, but we can set
  // up much of the work here.
  //
  // At first, let us declare a specializable template class that will hold the
  // arguments to `storage_kind`'s allocating constructor. The actual
  // specializations are defined in the preprocessed code.
  template<fragment Syntax, storage_type Storage>
  struct storage_alloc_args { };

  template<fragment Syntax, storage_type Storage>
  using storage_alloc_args_t = 
    typename storage_alloc_args<Syntax, Storage>::type;

  //
  // storage kinds with hierarchy elements must accept a type parameter before
  // the actual fields and children, so the first field of
  // `storage_alloc_args_t<>` must be a type, but not if there are no hierarchy
  // elements. We ensure that here with a maker trait that adds the needed type
  // on the top of those provided and returns a tuple of everything.
  //
  template<fragment Syntax, storage_type Storage, typename ...Types>
  struct make_storage_alloc_args { 
    using type = std::tuple<Types...>;
  };

  template<fragment Syntax, storage_type Storage, typename ...Types>
    requires storage_has_hierarchy_elements_v<Storage>
  struct make_storage_alloc_args<Syntax, Storage, Types...> { 
    using type = std::tuple<
      typename storage_type_of_t<Syntax, Storage>::type, Types...
    >;
  };

  //
  // Some hierarchy types can appear as arguments both for fields (e.g. the
  // `var` field of `quantifier` which is of type `variable`), and for children.
  // In the second case, since we must handle the fragments etc.., we store the
  // pointer to the underlying node. In the first case we do not. To distinguish
  // these cases, we declare here small wrapper types that will convert to the
  // appropriate type later.
  //
  template<hierarchy_type H, fragment Syntax>
  struct child_wrapper 
  {
    child_wrapper(hierarchy_base<H, Syntax> h) : child{h} { }

    operator hierarchy_node<H> const*() const {
      return child.node();
    }
    
    hierarchy_base<H, Syntax> child;
  };

  template<hierarchy_type H, fragment Syntax>
  struct children_wrapper
  {
    template<std::ranges::range R>
    using value_t = std::ranges::range_value_t<R>;

    template<std::ranges::range R>
      requires (value_t<R>::hierarchy == H && 
                is_subfragment_of_v<typename value_t<R>::syntax, Syntax>)
    children_wrapper(R v) {
      for(auto h : v)
        children.push_back(h.node());
    }

    operator std::vector<hierarchy_node<H> const*>() const {
      return children;
    }

    std::vector<hierarchy_node<H> const*> children;
  };


  //
  // To declare the allocating constructor, we need a trait to check whether a
  // given invocation is well formed. 
  //
  template<storage_type Storage, fragment Syntax, typename ...Args>
  inline constexpr bool is_storage_constructible_v = requires { 
    storage_alloc_args_t<Syntax, Storage>{std::declval<Args>()...}; 
  };

  template<storage_type Storage, fragment Syntax, typename ...Args>
  struct is_storage_constructible : std::bool_constant<
    is_storage_constructible_v<Storage, Syntax, Args...>
  > { };

  //
  // Similar thing for hierarchy elements, but a bit different because we need
  // to pass the type which does not come from the arguments.
  //
  template<syntax_element Element, fragment Syntax, typename ...Args>
  struct is_hierarchy_element_constructible
    : is_storage_constructible<
        storage_of_element_v<Element>, Syntax,
        type_value<Element>, Args...
      > { };

  template<syntax_element Element, fragment Syntax, typename ...Args>
  inline constexpr auto is_hierarchy_element_constructible_v = 
    is_hierarchy_element_constructible<Element, Syntax, Args...>::value;
  
  //
  // The same thing for leaf types is still a bit different. Here, since we do
  // not have children, we do not have to do any conversion between the argument
  // (e.g. `formula<LTL>`) and the actual value stored (e.g.
  // `hierarchy_node<hierarchy_type::formula>`). So we do not use
  // `storage_alloc_args_t`, and we just have to check if the arguments can
  // directly initialize the corresponding `storage_node`.
  //
  template<leaf_storage_kind Storage, typename ...Args>
  inline constexpr auto is_leaf_storage_constructible_v = requires { 
    storage_node<Storage::storage>{
      std::declval<syntax_element>(), std::declval<Args>()...
    }; 
  };

  template<leaf_storage_kind Storage, typename ...Args>
  struct is_leaf_storage_constructible
    : std::bool_constant<is_leaf_storage_constructible_v<Storage, Args...>> { };

  
  //
  // The allocating constructor needs to get access to the alphabet to make any
  // allocation. The alphabet is extracted with the following function from the
  // first argument that is a `hierarchy` or a range of `hierarchy`s. If no such
  // argument exists it means the hierarchy is a leaf and an instance has to be
  // requested directly to the alphabet (e.g. sigma.proposition("p")).
  //
  template<std::ranges::range R, typename ...Args>
    requires hierarchy<std::ranges::range_value_t<R>>
  alphabet *get_sigma(R const& r, Args ...) {
    black_assert(!empty(r));
    return begin(r)->sigma();
  }

  template<hierarchy T, typename ...Args>
  alphabet *get_sigma(T v, Args ...) {
      return v.sigma();
  }
  
  template<typename T, typename ...Args>
  alphabet *get_sigma(T, Args ...args) {
      return get_sigma(args...);
  }

  //
  // Once we got the arguments into the allocating constructor, we need to build
  // a `storage_node` from them. Since we wrapped the elements of
  // `storage_alloc_args_t` into suitable wrapper types `child_wrapper` and
  // `children_wrapper`, which have the right conversion operators, the
  // conversion is automatic, we just have to unpack the tuples correctly.
  //
  template<fragment Syntax, storage_type Storage>
    requires storage_has_hierarchy_elements_v<Storage>
  storage_node<Storage> args_to_node(
    storage_alloc_args_t<Syntax, Storage> const&args
  ) {
    return std::apply([](auto ...values) {
      return storage_node<Storage>{values...};
    }, args);
  }

  template<fragment Syntax, storage_type Storage>
    requires (!storage_has_hierarchy_elements_v<Storage>)
  storage_node<Storage> args_to_node(
    storage_alloc_args_t<Syntax, Storage> const&args
  ) {
    return std::apply([](auto ...values) {
      return storage_node<Storage>{element_of_storage_v<Storage>, values...};
    }, args);
  }

  //
  // Let us now address the remaining parts of the user interface of hierarchy
  // types. `storage_kind`s provides the user with member functions to access
  // fields and children (e.g. left() and right(), or label()). In the
  // preprocessing code, we only have the name of those fields, while the
  // contents are stored in tuples which can be accessed by index. Hence we need
  // a map from names of fields to indices in the tuples. This is implemented as
  // just an array of string views declared in the preprocessed code. Then, a
  // bunch of constexpr simbols holding the name of the single fields is
  // declared. Here, we only need to declare a function that searches for one
  // symbol in the table and returns its index. Note that the specialization of
  // the trait is selected with SFINAE instead of a `requires` clause because of
  // a bug in GCC 10.
  template<
    size_t I, std::string_view const*lits, const char *literal, typename = void
  >
  struct index_of_field : index_of_field<I + 1, lits, literal> { };

  template<size_t I, std::string_view const*lits, const char *literal>
  struct index_of_field<
    I, lits, literal, std::enable_if_t<lits[I] == literal>
  > : std::integral_constant<size_t, I> { };

  template<std::string_view const*lits, const char *literal>
  inline constexpr size_t index_of_field_v = 
    index_of_field<0, lits, literal>::value;

  //
  // Once we found which field to get, we just wrap it if needed. In order to
  // wrap child and children fields, we need to know their `hierarchy_type`.
  // This trait does this, specialized later.
  //
  template<size_t I, storage_type Storage>
  struct hierarchy_of_storage_child;
  
  template<size_t I, storage_type Storage>
  inline constexpr auto hierarchy_of_storage_child_v =
    hierarchy_of_storage_child<I, Storage>::value;
  
  //
  // Now we can actually get the fields.
  //
  template<size_t I, storage_kind H>
  auto get_field(H h) {
    return std::get<I>(h.node()->data.values);
  }

  template<size_t I, fragment Syntax, storage_kind H>
  auto get_child(H h) {
    using ChildH = 
      hierarchy_type_of_t<
        Syntax, hierarchy_of_storage_child_v<I, H::storage>
      >;
    return ChildH{h.sigma(), std::get<I>(h.node()->data.values)};
  }

  template<size_t I, fragment Syntax, storage_kind H>
  auto get_children(H h) {
    using ChildH = 
      hierarchy_type_of_t<
        Syntax, hierarchy_of_storage_child_v<I, H::storage>
      >;
    alphabet *sigma = h.sigma();
    auto children = std::get<I>(h.node()->data.values);

    std::vector<ChildH> result;
    for(auto child : children)
      result.push_back(ChildH{sigma, child});

    return result;
  }

  //
  // Another piece of public interface is the tuple-like access to hierarchy
  // types. This allows one to do something like `auto [l, r] = c;` to get left
  // and right children of a conjunction, and is also used by the pattern
  // matching infrastructure defined later.
  //
  // First, we need to know if the last child is a vector of children.
  // Specialized later.
  //
  template<size_t I, storage_type Storage>
  struct storage_ith_data_is_field : std::false_type { };
  
  template<size_t I, storage_type Storage>
  inline constexpr bool storage_ith_data_is_field_v =
    storage_ith_data_is_field<I, Storage>::value;
  
  template<size_t I, storage_type Storage>
  struct storage_ith_data_is_child : std::false_type { };
  
  template<size_t I, storage_type Storage>
  inline constexpr bool storage_ith_data_is_child_v =
    storage_ith_data_is_child<I, Storage>::value;

  //
  // The implementation of get<> calls get_field(), get_child() or
  // get_children() depending on the index requested.
  //
  template<size_t I, storage_kind S>
  auto get(S s) {
    if constexpr(storage_ith_data_is_field_v<I, S::storage>)
      return get_field<I>(s);
    else if constexpr(storage_ith_data_is_child_v<I, S::storage>)
      return get_child<I, typename S::syntax>(s);
    else 
      return get_children<I, typename S::syntax>(s);
  }

  //
  // Then we can declare the specializations.
  //
  } namespace std {

    template<black::internal::logic::storage_kind S>
    struct tuple_size<S> 
      : tuple_size<black::internal::logic::storage_data_t<S::storage>> { };

    template<size_t I, black::internal::logic::storage_kind S>
    struct tuple_element<I, S> {
      using type = decltype(get<I>(std::declval<S>()));
    };

  } namespace black::internal::logic {

  //
  // We also declare a function that, based on get<> above, makes a tuple of all
  // the fields of a hierarchy object. This is useful in combination with
  // `std::apply` which currently does not support Tuple-like user-defined
  // types.
  //
  template<storage_kind S, size_t ...I>
  auto as_tuple(S s, std::index_sequence<I...>) {
    return std::tuple{get<I>(s)...};
  }

  template<storage_kind S>
  auto as_tuple(S s) {
    return as_tuple(s, std::make_index_sequence<std::tuple_size_v<S>>{});
  }

  //
  // Before going into the implementation of the pattern matching
  // infrastructure, we implement the facility that it will use to compute the
  // return type of the `match()` function when the cases lambdas return
  // hierarchy types. We reuse the `std::common_type` trait, and we specialize
  // it for our types. The actual `std` specializations are in the preprocessed
  // source, since we have to provide it for actual user-defined types.
  //
  // The common type between two hierarchy types only exists of course if the
  // hierarchy is the same (e.g. formulas to formulas). The common type is the
  // hierarchy type with the combined fragment. Note that by the way
  // `make_combined_fragment_t` works, this handles the case where one of the
  // two fragments is subsumed by the other, returning directly the more general
  // one. Thus, `std::common_type_t<formula<LTL>, formula<Boolean>>` is
  // `formula<LTL>`, while `std::common_type_t<formula<LTL>, formula<FO>>` is
  // equivalent to `formula<LTLFO>`.
  template<typename T, typename U>
  struct common_type_helper { };

  //
  // Little helper template used later
  //
  template<typename T, typename H>
  struct have_same_storage : std::false_type { };
  
  template<storage_kind T, storage_kind H>
    requires (T::storage == H::storage)
  struct have_same_storage<T,H> : std::true_type { };

  template<typename T, typename H>
  inline constexpr bool have_same_storage_v = have_same_storage<T,H>::value;

  //
  // This is the common type of two hierarchies that are not storage kinds of
  // the same storage type.
  //
  template<hierarchy T, hierarchy U>
    requires (T::hierarchy == U::hierarchy && !have_same_storage_v<T, U>)
  struct common_type_helper<T, U> 
    : hierarchy_type_of<
        make_combined_fragment_t<typename T::syntax, typename U::syntax>,
        T::hierarchy
      > { };
  
  template<storage_kind T, storage_kind U>
    requires (T::storage == U::storage)
  struct common_type_helper<T, U> 
    : storage_type_of<
        make_combined_fragment_t<typename T::syntax, typename U::syntax>,
        T::storage
      > { };

  //
  // The actual specialization
  //
  } namespace std {

    template<
      black::internal::logic::hierarchy T, 
      black::internal::logic::hierarchy U
    > struct common_type<T, U>
      : black::internal::logic::common_type_helper<T, U> { };

  } namespace black::internal::logic {

  //
  // Since `fragment_type` is designed as well to be used in pattern matching,
  // we need to implement `common_type` for it and for `type_value` as well, so
  // to ease their use in pattern matching structures.
  //
  struct dummy_owner_t { }; 

  } namespace std {

    template<
      black::internal::logic::syntax_element E1, 
      black::internal::logic::syntax_element E2
    >
    struct common_type<
      black::internal::logic::type_value<E1>, 
      black::internal::logic::type_value<E2>
    > {
      using type = black::internal::logic::fragment_type<
        black::internal::logic::dummy_owner_t,
        black::internal::logic::syntax_list<E1, E2>
      >;
    };

    template<
      typename O, typename List, black::internal::logic::syntax_element E
    >
    struct common_type<
      black::internal::logic::fragment_type<O, List>,
      black::internal::logic::type_value<E>
    > {
      using type = black::internal::logic::fragment_type<O, 
        black::internal::logic::syntax_list_unique_t<
          black::internal::logic::syntax_list_concat_t<
            List, black::internal::logic::syntax_list<E>
          >
        >
      >;
    };

    template<
      typename O, typename List, black::internal::logic::syntax_element E
    >
    struct common_type<
      black::internal::logic::type_value<E>,
      black::internal::logic::fragment_type<O, List>
    > {
      using type = black::internal::logic::fragment_type<O, 
        black::internal::logic::syntax_list_unique_t<
          black::internal::logic::syntax_list_concat_t<
            List, black::internal::logic::syntax_list<E>
          >
        >
      >;
    };

    template<typename O1, typename L1,typename O2, typename L2>
    struct common_type<
      black::internal::logic::fragment_type<O1, L1>,
      black::internal::logic::fragment_type<O2, L2>
    > {
      using type = black::internal::logic::fragment_type<
        black::internal::logic::dummy_owner_t,
        black::internal::logic::syntax_list_unique_t<
          black::internal::logic::syntax_list_concat_t<L1, L2>
        >
      >;
    };

  } namespace black::internal::logic {

  //
  // Here we declare the infrastructure for pattern matching. The machinery is
  // based on the to<> function of hierarchy types, and on the list of
  // `syntax_element`s provided by the hierarchy's fragment. For each syntax
  // element we try to cast the formula to the corresponding type using
  // `to<>()`, calling the corresponding lambda case if the cast succeeds. This
  // is all made more complex because of the support for tuple-unpacking the
  // hierarchy type to the lambda arguments.
  //
  // The first thing we need is a function to do this std::apply-like unpacking
  // of the hierarchy to the called lambda. 

  template<typename Handler, typename T, size_t ...I>
    requires std::invocable<Handler, T, std::tuple_element_t<I, T>...>
  auto unpack(
    Handler&& handler, T h, std::index_sequence<I...>
  ) {
    return std::invoke(std::forward<Handler>(handler), h, get<I>(h)...);
  }

  //
  // It is cumbersome to repeat everything both in the body and in `decltype()`
  // but we need to remove the function from overload resolution if the handler
  // is not callable.
  //
  template<typename Handler, typename T>
  auto unpack(Handler&& handler, T h)
  -> decltype(
    unpack(
      std::forward<Handler>(handler), h, 
      std::make_index_sequence<std::tuple_size_v<T>>{}
    )
  ) {
    return unpack(
      std::forward<Handler>(handler), h, 
      std::make_index_sequence<std::tuple_size_v<T>>{}
    );
  }

  //
  // This trait holds if the call to `unpack` is well-formed
  //
  template<typename H, typename Handler>
  inline constexpr bool can_be_unpacked_v = requires { 
    unpack(std::declval<Handler>(), std::declval<H>()); 
  };

  //
  // The `dispatch()` function takes a hierarchy object and the list of handlers
  // and calls the first handler that can be called either directly or by
  // unpacking. Note that we check the unpacked case before to give it priority,
  // so that e.g. a lambda such as `[](conjunction, auto ...args) { }` picks up
  // the unpacked children in `args`.
  //
  template<typename T, typename Handler, typename ... Handlers>
  auto dispatch(T obj, Handler&& handler, Handlers&& ...handlers) 
  {
    if constexpr(can_be_unpacked_v<T, Handler>) 
      return unpack(std::forward<Handler>(handler), obj);
    else if constexpr(std::is_invocable_v<Handler, T>)
      return std::invoke(std::forward<Handler>(handler), obj);
    else 
      return dispatch(obj, std::forward<Handlers>(handlers)...);
  }

  //
  // Finally, the `matcher` class, which calls the machinery above to do the
  // trick. This is generic and works not only for hierarchy types but for any
  // type that expose `to<>` and `is<>` functions (e.g. `fragment_type` above).
  //
  // At first we need a concept to test the usability of such functions
  //
  template<typename H, typename Case>
  concept can_cast_to = requires(H h) {
    { h.template to<Case>() } -> std::convertible_to<std::optional<Case>>;
    { h.template is<Case>() } -> std::convertible_to<bool>;
  };

  //
  // Then, this utility trait is useful in the usage of the `matcher` class in
  // the common case of hierarchy types: it transform a `syntax_list` and a
  // fragment into a tuple of concrete hierarchy element types to pass to the
  // `matcher` class.
  //
  template<fragment Syntax, typename List>
  struct element_types_of_syntax_list;
  
  template<fragment Syntax, syntax_element ...Elements>
  struct element_types_of_syntax_list<Syntax, syntax_list<Elements...>> {
    using type = std::tuple<element_type_of_t<Syntax, Elements>...>;
  };

  template<fragment Syntax, typename List>
  using element_types_of_syntax_list_t = 
    typename element_types_of_syntax_list<Syntax, List>::type;

  //
  // Now the matcher class itself. The `H` paremeter is the main class from
  // which one wants to match (e.g. `formula<Syntax>`). The `Cases` parameter is
  // a tuple of types to try to match the matched object to. The default value
  // for `Cases` is the correct one to use in the common case of `H` being a
  // hierarchy type.
  //
  template<
    typename H,
    typename Cases = element_types_of_syntax_list_t<
      typename H::syntax,
      typename hierarchy_traits<H>::accepted_elements
    >
  >
  struct matcher;

  template<typename H, typename Case, typename ...Cases>
    //requires can_cast_to<H, Case>
  struct matcher<H, std::tuple<Case, Cases...>>
  {
    //
    // The return type of `match()` is computed with `std::common_type`, which
    // has been specialized for hierarchies above. Again, it is cumbersome to
    // repeat the body twice but there's no other way.
    //
    template<typename ...Handlers>
    static auto match(H h, Handlers&& ...handlers) 
      -> std::common_type_t<
        decltype(dispatch(
          *h.template to<Case>(), std::forward<Handlers>(handlers)...
        )),
        decltype(matcher<H, std::tuple<Cases...>>::match(
          h, std::forward<Handlers>(handlers)...
        ))
      >
    {
      if(h.template is<Case>())
        return dispatch(
          *h.template to<Case>(), std::forward<Handlers>(handlers)...
        );
      
      return matcher<H, std::tuple<Cases...>>::match(
        h, std::forward<Handlers>(handlers)...
      );
    }
  };

  //
  // The base case of the recursion above is the singleton list of cases. If we
  // reach this, and the main type cannot be casted to this last case, it means
  // it could not have been casted to any of the elements included in the list
  // of cases, which is clearly a bug, so we raise an assertion.
  //
  template<typename H, typename Case>
    //requires can_cast_to<H, Case>
  struct matcher<H, std::tuple<Case>>
  {
    template<typename ...Handlers>
    static auto match(H h, Handlers&& ...handlers) 
      -> decltype(dispatch(
        *h.template to<Case>(), std::forward<Handlers>(handlers)...
      )) 
    {
      if(h.template is<Case>())
        return dispatch(
          *h.template to<Case>(), std::forward<Handlers>(handlers)...
        );
      black_unreachable();
    }
  };

  //
  // Now we can implement the various `match()` functions declared until now.
  //
  // The first is for `fragment_type`.
  template<typename Elements>
  struct type_values_of_elements;

  template<syntax_element ...Elements>
  struct type_values_of_elements<syntax_list<Elements...>> {
    using type = std::tuple<type_value<Elements>...>;
  };

  template<typename Elements>
  using type_values_of_elements_t =
    typename type_values_of_elements<Elements>::type;

  template<typename Owner, typename List>
  template<typename ...Handlers>
  auto fragment_type<Owner, List>::match(Handlers ...hs) const {
    return matcher<fragment_type,
      type_values_of_elements_t<fragment_type::list>
    >{}.match(*this, hs...);
  }

  //
  // Then for hierarchy types
  //
  template<hierarchy_type H, fragment Syntax>
  template<typename ...Handlers>
  auto hierarchy_base<H, Syntax>::match(Handlers ...handlers) const {
    return matcher<hierarchy_base>{}.match(*this, handlers...);
  }

  template<storage_type S, fragment F, typename D>
  template<typename ...Handlers>
  auto storage_base<S, F, D>::match(Handlers ...handlers) const {
    return matcher<storage_base>{}.match(*this, handlers...);
  }

  template<syntax_element E, fragment S, typename D>
  template<typename ...Handlers>
  auto hierarchy_element_base<E, S, D>::match(Handlers ...handlers) const {
    return matcher<hierarchy_element_base>{}.match(*this, handlers...);
  }

  //
  // Little catch-all type used as a wildcard in patter matching 
  //
  struct otherwise {
    template<typename T>
    otherwise(T const&) { }
  };

  //
  // A useful addition to the pattern matching infrastructure is the `only<>`
  // class. It allows the user to match only a selected list of syntax elements
  // independently of their storage kind etc...
  //
  // First we define a trait to tell whether a fragment has only syntax elements
  // of the same hierarchy. The first template argument of `only<>` is of this
  // type.
  //
  template<typename List>
  struct are_uniform_elements : std::false_type { };

  template<syntax_element Element, syntax_element ...Elements>
  struct are_uniform_elements<syntax_list<Element, Elements...>>
    : std::bool_constant<
        ((hierarchy_of_storage_v<storage_of_element_v<Element>> == 
          hierarchy_of_storage_v<storage_of_element_v<Elements>>) && ...)> { };

  template<typename List>
  inline constexpr bool are_uniform_elements_v =
    are_uniform_elements<List>::value;

  template<typename Syntax>
  concept uniform_fragment = 
    fragment<Syntax> && are_uniform_elements_v<typename Syntax::list>;
  //
  // The following is the base hierarchy type that will be derived by `only`,
  // i.e. the hierarchy type that corresponds to the syntax elemnts in the
  // `TopLevel` fragment
  //
  template<uniform_fragment TopLevel, fragment Syntax>
  struct only_base 
    : hierarchy_type_of<Syntax, 
        hierarchy_of_storage_v<
          storage_of_element_v<
            syntax_list_head_v<typename TopLevel::list>
          >
        >
      > { };

  template<uniform_fragment TopLevel, fragment Syntax>
  using only_base_t = typename only_base<TopLevel, Syntax>::type;

  //
  // Then, the `only<>` class. The first argument is the fragment to which
  // `only<>` will restrict its matching. `Syntax` is the general fragment we
  // are considering, i.e. that of the children of the matched object.
  //
  // For example, `only<Future, formula<LTLP>` matches all the formulas of LTLP
  // which happens to have a future operator as the top level operator.
  // 
  template<uniform_fragment TopLevel, fragment Syntax>
  struct only : only_base_t<TopLevel, Syntax>
  {
    using base_t = only_base_t<TopLevel, Syntax>;

    only() = delete;
    only(only const&) = default;
    only(only &&) = default;

    only &operator=(only const&) = default;
    only &operator=(only &&) = default;

    //
    // Converting constructor. Remember that this will be called in the
    // `dispatch()` function above with an already-converted `hierarchy_element`
    // H. Hence, `H::accepted_elements` is a singleton, e.g.
    // `syntax_element::conjunction`, and we can know if the conversion is
    // successful at compile-time. Then, we also check the general fragment to
    // tell whether we are compatible.
    //
    template<hierarchy H>
      requires (H::hierarchy == base_t::hierarchy && 
        syntax_list_includes_v<
          typename TopLevel::list, 
          typename hierarchy_traits<H>::accepted_elements
        > && is_subfragment_of_v<typename H::syntax, Syntax>)
    only(H h) : base_t{h} { }


    //
    // Here we have some members to model the `hierarchy` concept
    //
    using syntax = Syntax;
    using accepts_type = make_syntax_predicate_t<typename TopLevel::list>;
    using type = typename Syntax::template type<only, accepts_type>;

    type node_type() {
      return type{this->node()->type};
    }

    //
    // We did all of this to come to this point: we call the `matcher` class but
    // passing only `TopLevel::list` as the list of syntax elements, so the
    // pattern matching is limited to those.
    //
    template<typename ...Handlers>
    auto match(Handlers ...handlers) const {
      return 
        matcher<base_t, 
          element_types_of_syntax_list_t<Syntax, typename TopLevel::list>
        >{}.match(*this, handlers...);
    }
  };

  //
  // Here we start defining a set of utilities to work with hierarchy types and
  // objects.
  //
  // The first is a function to orchestrate a generic recursion over all the
  // children of a hierarchy. When the concrete hierarchy is known this is
  // trivial, with a recursive function and the `match()` function, but when the
  // hierarchy type is generic, traversing all the children can be cumbersome.
  // With this function, it's easier. We apply a function to each child of the
  // hierarchy object, and that's it. We do not record the return value of the
  // function, because there is no way to return it back: in case of a known
  // storage kind, we know in particular which children are there and we could
  // return a tuple of the results, but when called on a generic hierarchy
  // object, there is no way to know which type to return. So the called
  // function must imperatively do something on its captured variables, but this
  // is not an heavy limitation.
  //
  // At first we define the overload of the function that takes a
  // `storage_kind`, since we know exactly which children there are in this
  // case. 
  template<size_t I, storage_type S, typename F>
  void for_each_child_aux(F) { }
  
  template<size_t I, storage_type S, typename Arg, typename ...Args, typename F>
  void for_each_child_aux(F f, Arg arg, Args...args) 
  {
    if constexpr(storage_ith_data_is_field_v<I, S>)
      for_each_child_aux<I+1, S>(f, args...);
    else if constexpr(storage_ith_data_is_child_v<I, S>) {
      f(arg);
      for_each_child_aux<I+1, S>(f, args...);
    } else {
      for(auto child : arg)
        f(child);
      for_each_child_aux<I+1, S>(f, args...);
    }
  }

  template<storage_kind S, typename F>
  void for_each_child(S s, F f) {
    std::apply([&](auto ...fields) {
      for_each_child_aux<0, S::storage>(f, fields...);
    }, as_tuple(s));
  }

  //
  // Then, we declare the overload that takes a `hierarchy`, which has to look
  // at which storage kind the hierarchy is in particular and call the
  // `storage_kind` overload consequently. This is done simply by reusing the
  // `match()` function. The lambda cases are called with the hierarchy
  // downcasted to the single hierarchy elements, so inside a generic lambda we
  // have the concrete type to call the `storage_kind` overload of the function.
  //
  template<hierarchy H, typename F>
  void for_each_child(H h, F f) {
    h.match(
      [&](auto s) {
        for_each_child(s, f);
      }
    );
  }

  //
  // The following function performs a cast between hierarchy types of the same
  // type but different fragments. It does no checks whatsoever to ensure the
  // conversion is legit, so it has to be used sparingly and only when the
  // programmer knows from other facts that the conversion is safe. Think of it
  // as a `static_cast` from base to derived: if you know that your Base *
  // pointer actually points to a Derived, then everything's ok, otherwise,
  // undefined behavior.
  //
  template<fragment Syntax, hierarchy H>
  auto fragment_unsafe_cast(H h) {
    return hierarchy_type_of_t<Syntax, H::hierarchy>{h.sigma(), h.node()};
  }
  
  template<fragment Syntax, storage_kind S>
  auto fragment_unsafe_cast(S s) {
    return storage_type_of_t<Syntax, S::storage>{s.sigma(), s.node()};
  }
  
  template<fragment Syntax, hierarchy_element E>
  auto fragment_unsafe_cast(E e) {
    return element_type_of_t<Syntax, E::element>{e.sigma(), e.node()};
  }

  //
  // `can_fragment_cast` checks whether `fragment_unsafe_cast` can be safely
  // called, by traversing the hierarchy object recursively to all the children.
  // So after a positive call of this function, `fragment_unsafe_cast` is safe
  // to use, but beware that this requires a complete traversal of the whole
  // hierarchy object, which for large formulas can be quite expensive. This is
  // a prototypical application of `for_each_child`.
  //
  template<fragment Syntax, hierarchy H>
  auto can_fragment_cast(H h) {
    if(!is_syntax_element_allowed(h.node_type(), typename Syntax::list{}))
      return false;

    bool can_cast = true;

    for_each_child(h, [&](auto child) {
      if(!can_fragment_cast<Syntax>(child))
        can_cast = false;
    });

    return can_cast;
  }

  //
  // This function is a checked version of `fragment_unsafe_cast` that returns
  // an empty optional if `can_fragment_cast` returns false. So this is safe to
  // use everywhere but beware of the same performance implications of
  // `can_fragment_cast`.
  //
  template<fragment Syntax, hierarchy H>
  auto fragment_cast(H h)
    -> std::optional<decltype(fragment_unsafe_cast<Syntax>(h))> 
  {
    if(can_fragment_cast<Syntax>(h))
      return {fragment_unsafe_cast<Syntax>(h)};
    return std::nullopt;
  }
}

#endif // BLACK_LOGIC_SUPPORT_HPP_
