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

#ifndef BLACK_INTERNAL_FORMULA_INTERFACE_HPP
#define BLACK_INTERNAL_FORMULA_INTERFACE_HPP

#include <black/support/assert.hpp>
#include <black/support/hash.hpp>

#include <tsl/hopscotch_map.h>

#include <cstdint>
#include <deque>

namespace black::internal::new_api 
{
  class alphabet;

  enum class hierarchy_type  : uint8_t {
    #define declare_hierarchy(Base) Base,
    #include <black/new/internal/formula/hierarchy.hpp>
  };

  enum class storage_type  : uint8_t {

    #define declare_storage_kind(Base, Storage) Storage,
    #include <black/new/internal/formula/hierarchy.hpp>

  };

  enum class syntax_element : uint8_t {
    #define declare_leaf_storage_kind(Base, Storage) Storage,
    #define has_no_hierarchy_elements(Base, Storage) Storage,
    #define declare_hierarchy_element(Base, Storage, Element) Element,

    #include <black/new/internal/formula/hierarchy.hpp>
  };

  #define declare_enum_element(Element) \
    template<> \
    struct pseudo_enum_element<syntax_element::Element> { \
      static constexpr pseudo_enum_value<syntax_element::Element> Element{}; \
    };

  #define declare_leaf_storage_kind(Base, Storage) declare_enum_element(Storage)
  #define has_no_hierarchy_elements(Base, Storage) declare_enum_element(Storage)
  #define declare_hierarchy_element(Base, Storage, Element) \
    declare_enum_element(Element)

  #include <black/new/internal/formula/hierarchy.hpp>

  #undef declare_enum_element

  #define declare_storage_kind(Base, Storage) \
    template<> \
    struct hierarchy_of_storage<storage_type::Storage> { \
      static constexpr auto value = hierarchy_type::Base; \
    };
  
  #include <black/new/internal/formula/hierarchy.hpp>
  
  #define declare_storage_of_element(Storage, Element) \
    template<> \
    struct storage_of_element<syntax_element::Element> { \
      static constexpr auto value = storage_type::Storage; \
    };
  
  #define declare_leaf_storage_kind(Base, Storage) \
    declare_storage_of_element(Storage, Storage)
  #define has_no_hierarchy_elements(Base, Storage) \
    declare_storage_of_element(Storage, Storage)
  #define declare_hierarchy_element(Base, Storage, Element) \
    declare_storage_of_element(Storage, Element)
  
  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_leaf_storage_kind(Base, Storage) \
  template<> \
  struct element_of_storage<storage_type::Storage> { \
    static constexpr auto value = syntax_element::Storage; \
  };

  #define has_no_hierarchy_elements(Base, Storage) \
    declare_leaf_storage_kind(Base, Storage)

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<fragment Syntax> \
    class Element;
  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    class Storage;
  #define declare_leaf_storage_kind(Base, Storage) \
    class Storage;
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    class Element;

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    struct element_type_of<Syntax, syntax_element::Element> { \
      using type = Element<Syntax>; \
    };
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    struct element_type_of<Syntax, syntax_element::Element> { \
      using type = Element; \
    };
  
  #define has_no_hierarchy_elements(Base, Storage) \
    declare_hierarchy_element(Base, Storage, Storage)

  #define declare_leaf_storage_kind(Base, Storage) \
    declare_leaf_hierarchy_element(Base, Storage, Storage)

  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // constexpr functions to categorize hierarchy type values into storage kinds
  //

  #define declare_hierarchy(Base) \
    template<> \
    struct hierarchy_syntax_predicate<hierarchy_type::Base> \
      : make_syntax_predicate_cpp<0 \

  #define declare_leaf_storage_kind(Base, Storage) \
          , syntax_element::Storage
  #define has_no_hierarchy_elements(Base, Storage) \
          , syntax_element::Storage
  #define declare_hierarchy_element(Base, Storage, Element) \
          , syntax_element::Element
    
  #define end_hierarchy(Base) \
        > { };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<> \
    struct storage_syntax_predicate<storage_type::Storage> \
      : make_syntax_predicate_cpp<0
      
  #define has_no_hierarchy_elements(Base, Storage) \
          , syntax_element::Storage

  #define declare_hierarchy_element(Base, Storage, Element) \
          , syntax_element::Element
    
  #define end_storage_kind(Base, Storage) \
          > { };

  #define declare_leaf_storage_kind(Base, Storage) \
    template<> \
    struct storage_syntax_predicate<storage_type::Storage> \
      : make_syntax_predicate<syntax_element::Storage> { };

  #define end_leaf_storage_kind(Base, Storage)
  
  #include <black/new/internal/formula/hierarchy.hpp>
  
  struct dummy_t {};

  #define declare_hierarchy(Base) \
    template<typename Derived> \
    struct Base##_custom_members_t :
  
  #define declare_custom_members(Base, Class) Class<Derived>, 

  #define end_hierarchy(Base) \
    dummy_t { };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy(Base) \
    template<fragment Syntax> \
    struct Base \
      : hierarchy_base<hierarchy_type::Base, Syntax>, \
        Base##_custom_members_t<Base<Syntax>> \
    { \
      using hierarchy_base<hierarchy_type::Base, Syntax>::hierarchy_base; \
    }; \
    \
    template<typename H> \
    Base(H const&) -> Base<typename H::syntax>;

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy(Base) \
    } namespace std { \
      template<typename Syntax> \
      struct hash<black::internal::new_api::Base<Syntax>>  {              \
        size_t operator()( \
          black::internal::new_api::Base<Syntax> const& t \
        ) const { \
          return t.hash();                \
        }                                                        \
      }; \
    } namespace black::internal::new_api {

  #include <black/new/internal/formula/hierarchy.hpp>

  #define has_no_standard_equality(Base) \
    template<> \
    struct hierarchy_has_standard_equality<hierarchy_type::Base> \
      : std::false_type { };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy(Base) \
    template<fragment Syntax> \
    struct hierarchy_type_of<Syntax, hierarchy_type::Base> { \
      using type = Base<Syntax>; \
    };
  
  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    struct storage_type_of<Syntax, storage_type::Storage> { \
      using type = Storage<Syntax>; \
    };
  
  #define declare_leaf_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    struct storage_type_of<Syntax, storage_type::Storage> { \
      using type = Storage; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>  

  #define declare_storage_kind(Base, Storage) \
    template<typename Derived> \
    struct storage_fields_base<storage_type::Storage, Derived> {

    #define declare_field(Base, Storage, Type, Field) \
      Type Field() const;

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename Derived, fragment Syntax> \
    struct storage_children_base<storage_type::Storage, Syntax, Derived> {

    #define declare_child(Base, Storage, Hierarchy, Child) \
      Hierarchy<Syntax> Child() const;

    #define declare_children(Base, Storage, Hierarchy, Children) \
      std::vector<Hierarchy<Syntax>> Children() const;

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define has_no_hierarchy_elements(Base, Storage) \
    template<> \
    struct storage_has_hierarchy_elements<storage_type::Storage> \
      : std::false_type { };

  #define declare_leaf_storage_kind(Base, Storage) \
    template<> \
    struct storage_has_hierarchy_elements<storage_type::Storage> \
      : std::false_type { };

  #include <black/new/internal/formula/hierarchy.hpp>

  template<typename T, typename = void, typename ...Args>
  struct is_aggregate_constructible_ : std::false_type { };

  template<typename T, typename ...Args>
  struct is_aggregate_constructible_<
    T, std::void_t<decltype(T{std::declval<Args>()...})>, Args...
  > : std::true_type { };

  template<typename T, typename ...Args>
  constexpr bool is_aggregate_constructible = 
    is_aggregate_constructible_<T, void, Args...>::value;

  template<typename T>
  concept can_get_fragment = hierarchy<T> || 
    (std::ranges::range<T> && hierarchy<std::ranges::range_value_t<T>>);

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
  
  template<typename ...Args>
  struct combined_fragment_from_args_ { };

  template<typename ...Args>
  using combined_fragment_from_args = 
    typename combined_fragment_from_args_<Args...>::type;

  template<can_get_fragment Arg>
  struct combined_fragment_from_args_<Arg> : get_fragment_from_arg<Arg> { };

  template<can_get_fragment Arg, typename ...Args>
  struct combined_fragment_from_args_<Arg, Args...> 
    : make_combined_fragment<
        get_fragment_from_arg_t<Arg>,
        combined_fragment_from_args<Args...>
      > { };
  
  template<typename Arg, typename ...Args>
  struct combined_fragment_from_args_<Arg, Args...>
    : combined_fragment_from_args_<Args...> { };

  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    class Storage : \
      public \
        storage_base<storage_type::Storage, Syntax, Storage<Syntax>>, \
      public Base##_custom_members_t<Storage<Syntax>> \
    { \
      using base_t = \
        storage_base<storage_type::Storage, Syntax, Storage<Syntax>>; \
    public: \
      using base_t::base_t; \
      \
      template<typename ...Args> \
        requires is_storage_constructible_v<storage_type::Storage, Syntax, Args...> \
      explicit Storage(Args ...args); \
    };\
    \
    template<typename H> \
    Storage(H const&) -> Storage<typename H::syntax>; \
  
  #define has_no_hierarchy_elements(Base, Storage) \
    template<typename ...Args> \
    explicit Storage(Args ...args) -> \
      Storage< \
        make_combined_fragment_t< \
          make_fragment_t<syntax_element::Storage>, \
          combined_fragment_from_args<Args...> \
        > \
      >;

  #define declare_leaf_storage_kind(Base, Storage) \
    class Storage : \
      public hierarchy_element_base< \
        syntax_element::Storage, make_fragment_t<syntax_element::Storage>, \
        Storage \
      >, \
      public Base##_custom_members_t<Storage> \
    { \
    public: \
      using hierarchy_element_base< \
        syntax_element::Storage, make_fragment_t<syntax_element::Storage>, \
        Storage \
      >::hierarchy_element_base; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<fragment Syntax> \
    class Element : \
      public hierarchy_element_base< \
        syntax_element::Element, Syntax, Element<Syntax> \
      >, \
      public Base##_custom_members_t<Element<Syntax>> \
    { \
      static_assert( \
        is_subfragment_of_v<make_fragment_t<syntax_element::Element>, Syntax>, \
        "'" #Element "' instance not allowed in its own syntax" \
      ); \
    public: \
      using hierarchy_element_base< \
        syntax_element::Element, Syntax, Element<Syntax> \
      >::hierarchy_element_base; \
      \
      template<typename ...Args> \
        requires is_hierarchy_element_constructible_v< \
          syntax_element::Element, Syntax, Args... \
        > \
      explicit Element(Args ...args); \
    }; \
    \
    template<typename ...Args> \
    explicit Element(Args ...args) -> \
      Element< \
        make_combined_fragment_t< \
          make_fragment_t<syntax_element::Element>, \
          combined_fragment_from_args<Args...> \
        > \
      >;
  
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    class Element : \
      public hierarchy_element_base< \
        syntax_element::Element, make_fragment_t<syntax_element::Element>, \
        Element \
      >, \
      public Base##_custom_members_t<Element> \
    { \
    public: \
      using hierarchy_element_base< \
        syntax_element::Element, make_fragment_t<syntax_element::Element>, \
        Element \
      >::hierarchy_element_base; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<> \
    struct storage_data<storage_type::Storage> \
      : make_storage_data<0

  #define declare_field(Base, Storage, Type, Field) , Type

  #define declare_child(Base, Storage, Hierarchy, Child) \
    , hierarchy_node<hierarchy_type::Hierarchy> const *
  
  #define declare_children(Base, Storage, Hierarchy, Children) \
    , std::vector<hierarchy_node<hierarchy_type::Hierarchy> const*>

  #define end_storage_kind(Base, Storage) \
      > { };

  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // tuple-like access
  //
  #define declare_storage_kind(Base, Storage) \
    constexpr size_t Storage##_arity() { \
      size_t arity = 0;

  #define declare_child(Base, Storage, Hierarchy, Child) \
      arity++;

  #define declare_children(Base, Storage, Hierarchy, Children) \
      arity++;

  #define end_storage_kind(Base, Storage) \
      return arity; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_leaf_storage_kind(Base, Storage) \
   } namespace std { \
      template<>                            \
      struct tuple_size<black::internal::new_api::Storage> \
        : std::integral_constant<int, 0> { }; \
    } namespace black::internal::new_api {

  #define declare_storage_kind(Base, Storage) \
    } namespace std { \
      template<typename Syntax>                            \
      struct tuple_size<black::internal::new_api::Storage<Syntax>> \
        : std::integral_constant< \
            int, black::internal::new_api::Storage##_arity() \
          > { }; \
      \
      template<size_t I, typename Syntax>                        \
      struct tuple_element<I, black::internal::new_api::Storage<Syntax>> {  \
        using type = black::internal::new_api::Base<Syntax>;             \
      }; \
    } namespace black::internal::new_api {

  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    } namespace std { \
      template<>                                \
      struct tuple_size<black::internal::new_api::Element> \
        : std::integral_constant<int, 0> { }; \
    } namespace black::internal::new_api {

  #define declare_hierarchy_element(Base, Storage, Element) \
    } namespace std { \
      template<typename Syntax>                                \
      struct tuple_size<black::internal::new_api::Element<Syntax>> \
        : std::integral_constant< \
            int, black::internal::new_api::Storage##_arity() \
          > { }; \
      \
      template<size_t I, typename Syntax>                                   \
      struct tuple_element<I, black::internal::new_api::Element<Syntax>> {  \
        using type = black::internal::new_api::Base<Syntax>;             \
      }; \
    } namespace black::internal::new_api {

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_leaf_storage_kind(Base, Storage)
  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    using Storage##_unpack_t = std::tuple<
  
  #define declare_child(Base, Storage, Hierarchy, Child) Hierarchy<Syntax>,
  #define declare_children(Base, Storage, Hierarchy, Child) \
    std::vector<Hierarchy<Syntax>>,
  
  #define end_storage_kind(Base, Storage) void *>;
  #define end_leaf_storage_kind(Base, Storage)

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_leaf_storage_kind(Base, Storage)
  #define declare_storage_kind(Base, Storage) \
    template<int I, typename Syntax, REQUIRES(I < Storage##_arity())> \
    Base<Syntax> get([[maybe_unused]] Storage<Syntax> s) {  \
      return std::get<I>(Storage##_unpack_t<Syntax>{

  #define declare_child(Base, Storage, Hierarchy, Child)  \
        s.Child(),
  #define declare_children(Base, Storage, Hierarchy, Children)  \
        s.Children(),
  
  #define end_leaf_storage_kind(Base, Storage)
  #define end_storage_kind(Base, Storage) \
        nullptr \
      }); \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_leaf_hierarchy_element(Base, Storage, Element)
  #define declare_hierarchy_element(Base, Storage, Element) \
    template<int I, typename Syntax, REQUIRES(I < Storage##_arity())> \
    Base<Syntax> get(Element<Syntax> e) { \
      return get<I>(Storage<Syntax>{e}); \
    }
  
  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // common_type
  //
  template<typename H, typename T, typename = void>
  struct common_type_helper;

  template<
    template<typename> class H1, template<typename> class H2,
    typename S1, typename S2
  >
  struct common_type_helper<
    H1<S1>, H2<S2>, std::enable_if_t<
      std::is_constructible_v<H1<make_combined_fragment_t<S1, S2>>, H2<S2>>
    >
  > { using type = H1<make_combined_fragment_t<S1, S2>>; };

  template<typename H, typename T, typename = void>
  struct common_type_leaf_helper;

  template<typename H, typename T>
  struct common_type_leaf_helper<
    H, T, std::enable_if_t<std::is_constructible_v<H, T>>
  > { using type = H; };
  
  template<typename T, typename = void>
  struct is_storage_ : std::false_type { };

  template<typename T>
  struct is_storage_<T, std::void_t<decltype(T::storage)>> 
    : std::true_type { };

  template<typename T>
  constexpr bool is_storage = is_storage_<T>::value;

  template<typename H, typename T, typename = void>
  struct common_type_different_helper;

  template<typename H1, typename H2>
  struct common_type_different_helper<
    H1, H2, std::enable_if_t<
      H1::hierarchy == H2::hierarchy &&
      H1::storage == H2::storage
    >
  > { 
    using type = storage_type_of_t<
      make_combined_fragment_t<typename H1::syntax, typename H2::syntax>,
      H1::storage
    >;
  };

  template<typename H1, typename H2>
  struct common_type_different_helper<
    H1, H2, std::enable_if_t<
      H1::hierarchy == H2::hierarchy &&
      H1::storage != H2::storage
    >
  > { 
    using type = hierarchy_type_of_t<
      make_combined_fragment_t<typename H1::syntax, typename H2::syntax>,
      H1::hierarchy
    >;
  };

  template<typename H1, typename H2>
  struct common_type_different_helper<
    H1, H2, std::enable_if_t<
      H1::hierarchy == H2::hierarchy &&
      (!is_storage<H1> || !is_storage<H2>)
    >
  > { 
    using type = hierarchy_type_of_t<
      make_combined_fragment_t<typename H1::syntax, typename H2::syntax>,
      H1::hierarchy
    >;
  };

  #define declare_ct_leaf(General, Specific) \
    template<typename Syntax> \
      struct common_type< \
        black::internal::new_api::General<Syntax>, \
        black::internal::new_api::Specific \
      > : black::internal::new_api::common_type_leaf_helper< \
          black::internal::new_api::General<Syntax>, \
          black::internal::new_api::Specific \
        > { }; \
      \
      template<typename Syntax> \
      struct common_type< \
        black::internal::new_api::Specific, \
        black::internal::new_api::General<Syntax> \
      > : black::internal::new_api::common_type_leaf_helper< \
          black::internal::new_api::General<Syntax>, \
          black::internal::new_api::Specific \
        > { };

  #define declare_ct(General, Specific) \
    template<typename Syntax1, typename Syntax2> \
    struct common_type< \
      black::internal::new_api::General<Syntax1>, \
      black::internal::new_api::Specific<Syntax2> \
    > : black::internal::new_api::common_type_helper< \
        black::internal::new_api::General<Syntax1>, \
        black::internal::new_api::Specific<Syntax2> \
      > { }; \
    \
    template<typename Syntax1, typename Syntax2> \
    struct common_type< \
      black::internal::new_api::Specific<Syntax1>, \
      black::internal::new_api::General<Syntax2> \
    > : black::internal::new_api::common_type_helper< \
        black::internal::new_api::General<Syntax2>, \
        black::internal::new_api::Specific<Syntax1> \
      > { };
  
  #define declare_ct_different(Element) \
    template<typename Syntax, typename T> \
    struct common_type< \
      black::internal::new_api::Element<Syntax>, T \
    > : black::internal::new_api::common_type_different_helper< \
        black::internal::new_api::Element<Syntax>, \
        T \
      > { };
  
  #define declare_ct_different_leaf(Element) \
    template<typename T> \
    struct common_type< \
      black::internal::new_api::Element, T \
    > : black::internal::new_api::common_type_different_helper< \
        black::internal::new_api::Element, \
        T \
      > { };

  #define declare_hierarchy(Base) \
    } namespace std {

  #define declare_leaf_storage_kind(Base, Storage) \
    declare_ct_leaf(Base, Storage) \
    declare_ct_different_leaf(Storage)
  
  #define declare_storage_kind(Base, Storage) \
    declare_ct(Base, Storage) \
    declare_ct_different(Storage)

  #define end_hierarchy(Base) \
    } namespace black::internal::new_api {

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    } namespace std {

  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    declare_ct_leaf(Storage, Element)

  #define declare_hierarchy_element(Base, Storage, Element) \
    declare_ct(Storage, Element) \
    declare_ct_different(Element)

  #define end_storage_kind(Base, Storage) \
    } namespace black::internal::new_api {

  #include <black/new/internal/formula/hierarchy.hpp>

  #undef declare_ct
  #undef declare_ct_leaf
}

#endif // BLACK_INTERNAL_FORMULA_INTERFACE_HPP
