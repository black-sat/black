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

//
// This file contains all the declarations that do not depend on including the
// hierarchy definition file, i.e. everything that does not need the
// preprocessor.
//

namespace black::internal::new_api {
  
  //
  // `alphabet` is the main factory class for all the formulas. See its
  // definition for more details.
  //
  class alphabet;
  
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
  template<syntax_element ...Elements>
  struct make_syntax_predicate_t { 
    static constexpr bool doesit(syntax_element e) {
      return ((e == Elements) || ...);
    }
  };

  template<syntax_element ...Elements>
  struct make_syntax_predicate {
    using type = make_syntax_predicate_t<Elements...>;
  };

  template<int dummy, syntax_element ...Elements>
  using make_syntax_predicate_cpp = make_syntax_predicate<Elements...>;

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
  // `fragment_pseudo_enum` models types used as pseudo-enum types for
  // enumerating hierarchy elements of storage kinds, e.g. `unary<LTL>::type`.
  // We can only just model the fact that the type holds a value of type
  // `syntax_element`. It tries to mimic being an actual enum class by being
  // explicitly convertible to its underlyng type.
  //
  template<typename T>
  concept fragment_pseudo_enum = requires(T t) {
    { syntax_element(t) };
    { uint8_t(t) };
  };
  
  //
  // The `fragment_type` class will provide the concrete instances of the
  // `fragment_pseudo_enum` concept. For a given `syntax_predicate` `P` and a
  // `syntax_list` `S`, `fragment_type<P, S>` will have one constexpr member
  // named exactly `Element` for each syntax element from `S` allowed by `P`.
  // For example, `unary<LTL>::type` has members `unary<LTL>::type::negation`,
  // `unary<LTL>::type::always`, etc... The concrete names will be injected by
  // the preprocessor later. Here, we define the needed types. That is why we
  // call it `pseudo enum`.
  //
  // We first declare a struct whose only purpose is to encapsulate a statically
  // known `syntax_element`.
  template<syntax_element Element>
  struct pseudo_enum_value {
    static constexpr syntax_element value = Element;
  };

  //
  // Then, an incomplete template class that will be specialized by the
  // preprocessor with the concrete names of the pseudo-enum values, e.g. we
  // will have:
  //
  // template<> 
  // struct pseudo_enum_element<syntax_element::conjunction> {
  //   static constexpr 
  //   pseudo_enum_value<syntax_element::conjunction> conjunction;
  // };
  template<syntax_element Element>
  struct pseudo_enum_element;

  //
  // Then, a class that derives from all the `pseudo_enum_element`s of a given
  // syntax list, used as a base class for `fragment_type`.
  //
  template<typename List>
  struct fragment_type_base;

  template<syntax_element ...Elements>
  struct fragment_type_base<syntax_list<Elements...>>
    : pseudo_enum_element<Elements>... { };

  //
  // Now we can define `fragment_type`, which derives from `fragment_type_base`
  // after filtering the list by the given predicate.
  //
  // The type itself is simple, and it only carries over the currently assigned
  // `syntax_element`. It can be only constructed by `pseudo_enum_value`s
  // corresponding to syntax elements included in its list.
  //
  template<syntax_predicate AcceptsType, typename List>
  struct fragment_type
    : fragment_type_base<syntax_list_filter_t<List, AcceptsType>> 
  {
    using list = syntax_list_filter_t<List, AcceptsType>;

    fragment_type() = delete;

    template<syntax_element Element>
      requires syntax_list_contains_v<list, Element>
    fragment_type(pseudo_enum_value<Element>) : _element{Element} { }

    explicit operator syntax_element() const { return _element; }
    explicit operator uint8_t() const { return uint8_t(_element); }

  private:
    syntax_element _element;
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
  //    instantiated,will give a `fragment_pseudo_enum`.
  //
  template<typename T>
  concept fragment = requires {
    requires is_syntax_list_v<typename T::list>;
    requires fragment_pseudo_enum<
      typename T::template type<false_syntax_predicate>
    >;
  };

  //
  // Now we can define actual fragments. To make a fragment made of some given
  // `syntax_element`s, just call `make_fragment_t<Elements...>`
  //
  template<syntax_element ...Elements>
  struct make_fragment_t {
    using list = syntax_list_unique_t<syntax_list<Elements...>>;
    
    template<syntax_predicate AcceptsType>
    using type = fragment_type<AcceptsType, list>;
  };

  template<syntax_element ...Elements>
  struct make_fragment {
    using type = make_fragment_t<Elements...>;
  };

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

    template<syntax_predicate AcceptsType>
    using type = fragment_type<AcceptsType, list>;
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
    requires fragment_pseudo_enum<typename T::type>;
    { T::hierarchy } -> std::convertible_to<hierarchy_type>;

    { t.unique_id() } -> 
      std::convertible_to<hierarchy_unique_id_t<T::hierarchy>>;
    { t.sigma() } -> std::convertible_to<alphabet *>;
    { t.hash() } -> std::convertible_to<size_t>;
    { t.node() } -> std::convertible_to<hierarchy_node<T::hierarchy> const *>;

    // to<H>() function to convert to another compatible hierarchy type H
    // is<H>() function to tell if it can be converted to H
    // match(...) function for pattern matching
  };

  //
  // This trait type provides information computed from the given `hierarchy`
  // type.
  // 1. `syntax` and `accepts_type` are just exported from the hierarchy type.
  // 2. `accepted_elements` is the syntax list from `syntax` filtered by
  //    `accepts_type`.
  // 3. `unique_id_t` is the unique type id from `hierarchy_unique_id_t`
  //
  template<hierarchy H>
  struct hierarchy_traits {
    using syntax = typename H::syntax;
    using accepts_type = typename H::accepts_type;
    using accepted_elements =
      syntax_list_filter_t<typename syntax::list, typename H::accepts_type>;
    using unique_id_t = hierarchy_unique_id_t<H::hierarchy>;
  };

  //
  // Similarly to hierarchy_node<>, we have storage_node<> as a concrete node
  // type for storage kinds. Its members however must be personalized for each
  // storage kind from the definitions file, so we declare here an empty data
  // class that will have to be specialized later.
  //
  template<storage_type Storage>
  struct storage_data_t;

  template<storage_type Storage>
  struct storage_node : hierarchy_node<hierarchy_of_storage_v<Storage>>
  { 
    storage_node(syntax_element element, storage_data_t<Storage> const&_data)
      : hierarchy_node<hierarchy_of_storage_v<Storage>>{element},
        data{_data} { }

    storage_data_t<Storage> data;
  };

  //
  // This concept refines `hierarchy` for storage kinds.
  //
  template<typename T>
  concept storage_kind = hierarchy<T> && requires(T t) {
    { T::storage } -> std::convertible_to<storage_type>;
    { t.node() } -> std::convertible_to<storage_node<T::storage> const *>;
  };

  //
  // This concept refines `storage_kind` for hierarchy elements.
  //
  template<typename T>
  concept hierarchy_element = storage_kind<T> && requires(T t) {
    { T::element } -> std::convertible_to<syntax_element>;
  };

  //
  // We need to forward-declare the `matcher` template class used for pattern 
  // matching, defined later.
  //
  template<
    hierarchy H, 
    fragment Syntax = typename H::syntax, 
    typename Cases = typename hierarchy_traits<H>::accepted_elements
  >
  struct matcher;

  //
  // We can now declare the base class for all the hierarchy types. This class
  // provides all the necessary infrastructure to obtain a complete hierarchy
  // type. Concrete hierarchy types are obtained by preprocessing the hierarchy
  // definition file by just deriving from this base class and re-exporting the
  // constructors.
  //
  template<hierarchy_type Hierarchy, fragment Syntax>
  class hierarchy_base {
    using node_t = hierarchy_node<Hierarchy>;
  public:
    // members required by the `hierarchy` concept
    using syntax = Syntax;
    using accepts_type = hierarchy_syntax_predicate_t<Hierarchy>;
    using type = typename Syntax::template type<accepts_type>;
    static constexpr auto hierarchy = Hierarchy;

    // hierarchy types are not default constructible but are
    // copy/move/constructible/assignable
    hierarchy_base() = delete;
    hierarchy_base(hierarchy_base const&) = default;
    hierarchy_base(hierarchy_base &&) = default;

    hierarchy_base &operator=(hierarchy_base const&) = default;
    hierarchy_base &operator=(hierarchy_base &&) = default;

    // this constructor is for internal use but has to be public (for now)
    hierarchy_base(alphabet *sigma, node_t const*node)
      : _sigma{sigma}, _node{node} { }

    // converting constructor from other hierarchy types
    // the conversion only happen for the same kind of hierarchy (e.g. formulas)
    // and only if the argument's syntax is a subfragment of our syntax
    template<::black::internal::new_api::hierarchy H>
      requires (H::hierarchy == Hierarchy && 
                is_subfragment_of_v<typename H::syntax, syntax>)
    hierarchy_base(H h) : hierarchy_base{h.sigma(), h.node()} { }

    //
    // other member functions
    //
    template<::black::internal::new_api::hierarchy H>
    std::optional<H> to() const {
      return H::from(*this);
    }

    template<::black::internal::new_api::hierarchy H>
    bool is() const {
      return to<H>().has_value();
    }

    template<typename ...Handlers>
    auto match(Handlers ...handlers) const {
      return matcher<hierarchy_base>{}.match(*this, handlers...);
    }

    auto unique_id() const {
      return hierarchy_unique_id_t<hierarchy>{
        reinterpret_cast<uintptr_t>(_node)
      };
    }

    size_t hash() const {
      return std::hash<hierarchy_node<hierarchy> const*>{}(_node);\
    }
    
    alphabet *sigma() const { return _sigma; }
    auto node() const { return _node; }

  private:
    alphabet *_sigma;
    node_t const*_node;
  };
  

}

#endif // BLACK_LOGIC_SUPPORT_HPP_
