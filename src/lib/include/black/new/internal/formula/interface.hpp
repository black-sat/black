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

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    class Element;
  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    class Storage;
  #define declare_leaf_storage_kind(Base, Storage) \
    class Storage;
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    class Element;

  #include <black/new/internal/formula/hierarchy.hpp>

  template<typename Syntax, syntax_element Element>
  struct type_for_syntax_element_;

  template<typename Syntax, syntax_element Element>
  using type_for_syntax_element = 
    typename type_for_syntax_element_<Syntax, Element>::type;

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    struct type_for_syntax_element_<Syntax, syntax_element::Element> { \
      using type = Element<Syntax>; \
    };
  
  #define has_no_hierarchy_elements(Base, Storage) \
    declare_hierarchy_element(Base, Storage, Storage)
    
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    struct type_for_syntax_element_<Syntax, syntax_element::Element> { \
      using type = Element; \
    };

  #define declare_leaf_storage_kind(Base, Storage) \
    declare_leaf_hierarchy_element(Base, Storage, Storage)

  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // constexpr functions to categorize hierarchy type values into storage kinds
  //

  #define declare_hierarchy(Base) \
    template<> \
    struct hierarchy_syntax_predicate<hierarchy_type::Base> \
      : make_syntax_predicate<0 \

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
      : make_syntax_predicate<0
      
  #define has_no_hierarchy_elements(Base, Storage) \
          , syntax_element::Storage

  #define declare_hierarchy_element(Base, Storage, Element) \
          , syntax_element::Element
    
  #define end_storage_kind(Base, Storage) \
          > { };

  #define declare_leaf_storage_kind(Base, Storage) \
    template<> \
    struct storage_syntax_predicate<storage_type::Storage> \
      : make_syntax_predicate<0, syntax_element::Storage> { };

  #define end_leaf_storage_kind(Base, Storage)
  
  #include <black/new/internal/formula/hierarchy.hpp>


  #define declare_hierarchy_element(Base, Storage, Element) \
    struct Element##_accepts_type { \
      static constexpr bool doesit(syntax_element type) { \
        return type == syntax_element::Element; \
      } \
    };

  #include <black/new/internal/formula/hierarchy.hpp>
  
  struct dummy_t {};

  #define declare_hierarchy(Base) \
    template<typename Derived> \
    struct Base##_custom_members_t :
  
  #define declare_custom_members(Base, Class) Class<Derived>, 

  #define end_hierarchy(Base) \
    dummy_t { };

  #include <black/new/internal/formula/hierarchy.hpp>

  template<
    typename H, 
    typename Syntax = typename H::syntax, 
    typename Cases = typename hierarchy_traits<H>::accepted_elements
  >
  struct matcher;

  #define declare_hierarchy(Base) \
    template<fragment Syntax> \
    class Base : public Base##_custom_members_t<Base<Syntax>> \
    { \
    public: \
      using syntax = Syntax; \
      using accepts_type = hierarchy_syntax_predicate_t<hierarchy_type::Base>; \
      using type = typename Syntax::template type<accepts_type>; \
      static constexpr auto hierarchy = hierarchy_type::Base; \
      \
      Base() = delete; \
      Base(Base const&) = default; \
      Base(Base &&) = default; \
      \
      Base &operator=(Base const&) = default; \
      Base &operator=(Base &&) = default; \
      \
      Base(alphabet *sigma, hierarchy_base<hierarchy_type::Base> *element) \
        : _sigma{sigma}, _element{element} { } \
      \
      template< \
        typename H, \
        REQUIRES( \
          H::hierarchy == hierarchy_type::Base && \
          is_subfragment_of_v<typename H::syntax, Syntax> \
        ) \
      > \
      Base(H const& h); \
      \
      template<typename H> \
      std::optional<H> to() const { \
        return H::from(*this); \
      } \
      \
      template<typename H> \
      bool is() const { \
        return to<H>().has_value(); \
      } \
      \
      auto unique_id() const { \
        return hierarchy_unique_id_t<hierarchy>{ \
          reinterpret_cast<uintptr_t>(_element) \
        }; \
      } \
      \
      template<typename ...Handlers> \
      auto match(Handlers ...handlers) const { \
        return matcher<Base>{}.match(*this, handlers...); \
      } \
      \
      type syntax_element() const { return _element->type; } \
      \
      alphabet *sigma() const { return _sigma; }  \
      \
      size_t hash() const { \
        return std::hash<hierarchy_base<hierarchy_type::Base> const*>{}(_element); \
      } \
      \
      alphabet *_sigma; \
      hierarchy_base<hierarchy_type::Base> *_element; \
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

  template<typename T, typename = void>
  struct is_hierarchy_ : std::false_type { };

  template<typename T>
  struct is_hierarchy_<T, std::void_t<decltype(T::hierarchy)>> 
    : std::true_type { };

  template<typename T>
  constexpr bool is_hierarchy = is_hierarchy_<T>::value;

  template<
    typename T1, typename T2, 
    REQUIRES(is_hierarchy<T1> && is_hierarchy<T2>)
  >
  bool are_equal(T1 t1, T2 t2) {
    return t1.unique_id() == t2.unique_id();
  }

  template<
    typename T1, typename T2, 
    REQUIRES(!is_hierarchy<T1> || !is_hierarchy<T2>)
  >
  bool are_equal(T1 t1, T2 t2) {
    return t1 == t2;
  }

  #define has_standard_equality(Base) \
    template< \
      typename H1, typename H2, \
      REQUIRES( \
        H1::hierarchy == hierarchy_type::Base && \
        H2::hierarchy == hierarchy_type::Base \
      ) \
    > \
    bool operator==(H1 h1, H2 h2) { \
      return are_equal(h1, h2); \
    } \
    \
    template< \
      typename H1, typename H2, \
      REQUIRES( \
        H1::hierarchy == hierarchy_type::Base && \
        H2::hierarchy == hierarchy_type::Base \
      ) \
    > \
    bool operator!=(H1 h1, H2 h2) { \
      return !are_equal(h1, h2); \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // internal representation classes for each storage kind
  //
  #define declare_storage_kind(Base, Storage) \
    struct Storage##_data_t;

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_t;

  #include <black/new/internal/formula/hierarchy.hpp>
  
  template<hierarchy_type H>
  struct hierarchy_base_type_of_;

  template<hierarchy_type H>
  using hierarchy_base_type_of = typename hierarchy_base_type_of_<H>::type;
  
  #define declare_hierarchy(Base) \
    template<> \
    struct hierarchy_base_type_of_<hierarchy_type::Base> { \
      using type = hierarchy_base<hierarchy_type::Base>; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  template<typename Syntax, hierarchy_type H>
  struct hierarchy_type_of_;

  template<typename Syntax, hierarchy_type H>
  using hierarchy_type_of = 
    typename hierarchy_type_of_<Syntax, H>::type;
  
  #define declare_hierarchy(Base) \
    template<typename Syntax> \
    struct hierarchy_type_of_<Syntax, hierarchy_type::Base> { \
      using type = Base<Syntax>; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  template<storage_type H>
  struct storage_base_type_of_;

  template<storage_type H>
  using storage_base_type_of = typename storage_base_type_of_<H>::type;
  
  #define declare_storage_kind(Base, Storage) \
    template<> \
    struct storage_base_type_of_<storage_type::Storage> { \
      using type = Storage##_t; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  template<typename Syntax, storage_type H>
  struct storage_type_of_;

  template<typename Syntax, storage_type H>
  using storage_type_of = typename storage_type_of_<Syntax, H>::type;
  
  #define declare_leaf_storage_kind(Base, Storage) \
    template<typename Syntax> \
    struct storage_type_of_<Syntax, storage_type::Storage> { \
      using type = Storage; \
    };
  
  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    struct storage_type_of_<Syntax, storage_type::Storage> { \
      using type = Storage<Syntax>; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>
  
  //
  // Handle classes.
  // 
  // The definition of an handle is split in pieces because of the macros.
  // - The Storage##_fields CRTP class declare the fields accessors
  // - The Storage class is the handle
  //
  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename H> \
    struct Storage##_fields {

    #define declare_field(Base, Storage, Type, Field) \
      Type Field() const;

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax, typename H> \
    struct Storage##_children {

  #define declare_child(Base, Storage, Hierarchy, Child) \
      Hierarchy<Syntax> Child() const;

  #define declare_children(Base, Storage, Hierarchy, Children) \
      std::vector<Hierarchy<Syntax>> Children() const;

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    constexpr bool Storage##_has_hierarchy_elements() { \
      return 
  
  #define has_no_hierarchy_elements(Base, Storage) false &&

  #define end_storage_kind(Base, Storage) \
      true; \
    }

  #define declare_leaf_storage_kind(Base, Storage) \
    constexpr bool Storage##_has_hierarchy_elements() { \
      return false; \
    }

  #define end_leaf_storage_kind(Base, Storage)

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    struct Storage##_alloc_args;

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

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax, typename ...Args> \
    constexpr bool is_##Storage##_constructible = \
      is_aggregate_constructible< \
        Storage##_alloc_args<Syntax>, int, Args... \
      >;

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax, typename ...Args> \
    constexpr bool is_##Element##_constructible = \
      is_aggregate_constructible< \
        Storage##_alloc_args<Syntax>, \
        int, decltype(Storage<Syntax>::type::Element), \
        Args... \
      >;

  #include <black/new/internal/formula/hierarchy.hpp>

  
  template<typename Derived, bool Leaf>
  struct storage_common_interface
  {
    template<typename H>
    std::optional<H> to() const {
      return H::from(self());
    }
   
    template<typename H>
    bool is() const {
      return to<H>().has_value();
    }
   
    size_t hash() const {
      using storage_t = storage_base_type_of<Derived::storage>;
      return std::hash<storage_t const*>{}(self()._element);
    }
   
    alphabet *sigma() const { return self()._sigma; }
    auto unique_id() const {
      return 
        hierarchy_unique_id_t<Derived::hierarchy>{
          reinterpret_cast<uintptr_t>(self()._element)
        };
    }
   
    template<
      typename F, typename D = Derived,
      REQUIRES(F::hierarchy == D::hierarchy)
    >
    static std::optional<Derived> from(F f) {
      using storage_t = storage_base_type_of<Derived::storage>;
      using accepts_type = typename Derived::accepts_type;

      if constexpr(
        !Leaf && 
        !is_subfragment_of_v<typename F::syntax, typename Derived::syntax>
      ) return {};

      if(!accepts_type::doesit(f._element->type))
        return {};

      auto obj = static_cast<storage_t *>(f._element);
      return std::optional<Derived>{Derived{f._sigma, obj}};
    }
 
  protected:
    Derived &self() { return static_cast<Derived &>(*this); }
    Derived const&self() const { return static_cast<Derived const&>(*this); }
  };

  template<typename Arg, typename = void>
  struct can_get_syntax_from_arg_ : std::false_type { };

  template<typename Arg>
  struct can_get_syntax_from_arg_<Arg, std::enable_if_t<is_hierarchy<Arg>>>
    : std::true_type { };
  
  template<typename Arg>
  struct can_get_syntax_from_arg_<
    Arg, std::enable_if_t<is_hierarchy<typename Arg::value_type>>
  > : std::true_type { };

  template<typename Arg>
  constexpr bool can_get_syntax_from_arg =
    can_get_syntax_from_arg_<Arg>::value;

  template<typename Arg, typename = void>
  struct get_syntax_from_arg_ { };

  template<typename Arg>
  struct get_syntax_from_arg_<Arg, std::enable_if_t<is_hierarchy<Arg>>> { 
    using type = typename Arg::syntax;
  };
  
  template<typename Arg>
  struct get_syntax_from_arg_<
    Arg, std::enable_if_t<is_hierarchy<typename Arg::value_type>>
  > {
    using type = typename Arg::value_type::syntax;
  };

  template<typename Arg>
  using get_syntax_from_arg = typename get_syntax_from_arg_<Arg>::type;
  
  template<typename, typename ...Args>
  struct combined_fragment_from_args_ { };

  template<typename ...Args>
  using combined_fragment_from_args = 
    typename combined_fragment_from_args_<void, Args...>::type;

  template<typename Arg>
  struct combined_fragment_from_args_<
    std::enable_if_t<can_get_syntax_from_arg<Arg>>, Arg
  > {
    using type = get_syntax_from_arg<Arg>;
  };

  template<typename Arg, typename ...Args>
  struct combined_fragment_from_args_<
    std::enable_if_t<can_get_syntax_from_arg<Arg>>,
    Arg, Args...
  > {
    using type = make_combined_fragment_t<
      get_syntax_from_arg<Arg>,
      combined_fragment_from_args<Args...>
    >;
  };
  
  template<typename Arg, typename ...Args>
  struct combined_fragment_from_args_<
    std::enable_if_t<!can_get_syntax_from_arg<Arg>>,
    Arg, Args...
  > {
    using type = combined_fragment_from_args<Args...>;
  };

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    class Storage : \
      public Storage##_fields<Storage<Syntax>>, \
      public Storage##_children<Syntax, Storage<Syntax>>, \
      public storage_common_interface<Storage<Syntax>, false>, \
      public Base##_custom_members_t<Storage<Syntax>> \
    { \
      friend struct Storage##_fields<Storage<Syntax>>; \
      friend struct Storage##_children<Syntax, Storage<Syntax>>; \
    public: \
      using accepts_type = storage_syntax_predicate_t<storage_type::Storage>; \
      using syntax = Syntax; \
      using type = typename Syntax::template type<accepts_type>; \
      static constexpr auto hierarchy = hierarchy_type::Base; \
      static constexpr auto storage = storage_type::Storage; \
      \
      Storage(Storage const&) = default; \
      Storage(Storage &&) = default; \
      \
      Storage &operator=(Storage const&) = default; \
      Storage &operator=(Storage &&) = default; \
      \
      Storage(class alphabet *sigma, Storage##_t *element) \
        : _sigma{sigma}, _element{element} { } \
      \
      template< \
        typename H, \
        REQUIRES( \
          H::storage == storage_type::Storage && \
          is_subfragment_of_v<typename H::syntax, Syntax> \
        ) \
      > \
      Storage(H const&e); \
      \
      template< \
        typename ...Args, \
        REQUIRES(is_##Storage##_constructible<Syntax, Args...>) \
      > \
      explicit Storage(Args ...args); \
      \
      template<typename ...Handlers> \
      auto match(Handlers ...handlers) const \
      { \
        return matcher<Storage>{}.match(*this, handlers...); \
      } \
      \
      class alphabet *_sigma; \
      Storage##_t *_element; \
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
      public Storage##_fields<Storage>, \
      public storage_common_interface<Storage, true>, \
      public Base##_custom_members_t<Storage> \
    { \
      \
      friend struct Storage##_fields<Storage>; \
    public: \
      using accepts_type = storage_syntax_predicate_t<storage_type::Storage>; \
      using syntax = make_fragment_t<syntax_element::Storage>; \
      using type = syntax::template type<accepts_type>; \
      static constexpr auto hierarchy = hierarchy_type::Base; \
      static constexpr auto storage = storage_type::Storage; \
      \
      Storage(Storage const&) = default; \
      Storage(Storage &&) = default; \
      \
      Storage(class alphabet *sigma, Storage##_t *element) \
        : _sigma{sigma}, _element{element} { } \
      \
      Storage &operator=(Storage const&) = default; \
      Storage &operator=(Storage &&) = default; \
      \
      class alphabet *_sigma; \
      Storage##_t *_element; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    class Element : \
      public Storage##_fields<Element<Syntax>>, \
      public Storage##_children<Syntax, Element<Syntax>>, \
      public storage_common_interface<Element<Syntax>, false>, \
      public Base##_custom_members_t<Element<Syntax>> \
    { \
      friend struct Storage##_fields<Element<Syntax>>; \
      friend struct Storage##_children<Syntax, Element<Syntax>>; \
      static_assert( \
        is_subfragment_of_v<make_fragment_t<syntax_element::Element>, Syntax>, \
        "'" #Element "' instance not allowed in its own syntax" \
      ); \
    public: \
      using accepts_type = Element##_accepts_type; \
      using syntax = Syntax; \
      using type = typename Syntax::template type<accepts_type>; \
      static constexpr auto hierarchy = hierarchy_type::Base; \
      static constexpr auto storage = storage_type::Storage; \
      static constexpr auto element = syntax_element::Element; \
      \
      Element(Element const&) = default; \
      Element(Element &&) = default; \
      \
      Element(class alphabet *sigma, Storage##_t *element); \
      \
      template<typename S, REQUIRES(is_subfragment_of_v<S, Syntax>)> \
      Element(Element<S> e); \
      \
      template< \
        typename ...Args, \
        REQUIRES(is_##Element##_constructible<Syntax, Args...>) \
      > \
      explicit Element(Args ...args); \
      \
      Element &operator=(Element const&) = default; \
      Element &operator=(Element &&) = default; \
      \
      class alphabet *_sigma; \
      Storage##_t *_element; \
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
      public Storage##_fields<Element>, \
      public storage_common_interface<Element, true>, \
      public Base##_custom_members_t<Element> \
    { \
      friend struct Storage##_fields<Element>; \
    public: \
      using accepts_type = Element##_accepts_type; \
      using syntax = make_fragment_t<syntax_element::Element>; \
      using type = syntax::template type<accepts_type>; \
      static constexpr auto hierarchy = hierarchy_type::Base; \
      static constexpr auto storage = storage_type::Storage; \
      static constexpr auto element = syntax_element::Element; \
      \
      Element(Element const&) = default; \
      Element(Element &&) = default; \
      \
      Element(class alphabet *sigma, Storage##_t *element); \
      \
      Element &operator=(Element const&) = default; \
      Element &operator=(Element &&) = default; \
      \
      class alphabet *_sigma; \
      Storage##_t *_element; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    } namespace std { \
      template<typename Syntax> \
      struct hash<black::internal::new_api::Storage<Syntax>>  {              \
        size_t operator()( \
          black::internal::new_api::Storage<Syntax> const& t \
        ) const { \
          return t.hash();                \
        }                                                        \
      }; \
    } namespace black::internal::new_api {
  
  #define declare_hierarchy_element(Base, Storage, Element) \
    } namespace std { \
      template<typename Syntax> \
      struct hash<black::internal::new_api::Element<Syntax>>  {              \
        size_t operator()( \
          black::internal::new_api::Element<Syntax> const& t \
        ) const { \
          return t.hash();                \
        }                                                        \
      }; \
    } namespace black::internal::new_api {

  #define declare_leaf_storage_kind(Base, Storage) \
    } namespace std { \
      template<> \
      struct hash<black::internal::new_api::Storage>  {              \
        size_t operator()( \
          black::internal::new_api::Storage const& t \
        ) const { \
          return t.hash();                \
        }                                                        \
      }; \
    } namespace black::internal::new_api {
  
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    } namespace std { \
      template<> \
      struct hash<black::internal::new_api::Element>  {              \
        size_t operator()( \
          black::internal::new_api::Element const& t \
        ) const { \
          return t.hash();                \
        }                                                        \
      }; \
    } namespace black::internal::new_api {

  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // tuple-like access
  //
  #define declare_storage_kind(Base, Storage) \
    constexpr size_t Storage##_arity() { \
      size_t arity = 0;

  #define declare_child(Base, Storage, Hierarchy, Child) \
      arity++;

  #define declare_children(Base, Storage, Hierarchy, Child) \
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
    using type = storage_type_of<
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
    using type = hierarchy_type_of<
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
    using type = hierarchy_type_of<
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
