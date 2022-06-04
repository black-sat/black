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

  enum class hierarchy_type : uint8_t {
    #define declare_hierarchy(Base) Base,
    #include <black/new/internal/formula/hierarchy.hpp>
  };

  //
  // enum type of all the elements of the hierarchies
  //
  enum class syntax_element : uint8_t {

    no_type,

    #define declare_leaf_storage_kind(Base, Storage) Storage,
    #define has_no_hierarchy_elements(Base, Storage) Storage,
    #define declare_hierarchy_element(Base, Storage, Element) Element,

    #include <black/new/internal/formula/hierarchy.hpp>

  };

  template<syntax_element ...>
  struct make_fragment;

  //
  // Base class for internal representation of elements of the hierarchies
  //
  #define declare_hierarchy(Base) \
  struct Base##_base { \
    syntax_element type; \
  };

  #include <black/new/internal/formula/hierarchy.hpp>

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
    struct Base##_accepts_type { \
      static constexpr bool doesit(syntax_element type) { \
        return 

  #define declare_leaf_storage_kind(Base, Storage) \
          type == syntax_element::Storage ||
  #define has_no_hierarchy_elements(Base, Storage) \
          type == syntax_element::Storage ||
  #define declare_hierarchy_element(Base, Storage, Element) \
          type == syntax_element::Element ||
    
  #define end_hierarchy(Base) \
        false; \
      } \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_accepts_type { \
      static constexpr bool doesit(syntax_element type) { \
        return   

  #define has_no_hierarchy_elements(Base, Storage) \
          type == syntax_element::Storage ||

  #define declare_hierarchy_element(Base, Storage, Element) \
          type == syntax_element::Element ||
    
  #define end_storage_kind(Base, Storage) \
          false; \
      } \
    };

  #define declare_leaf_storage_kind(Base, Storage) \
    struct Storage##_accepts_type { \
      static constexpr bool doesit(syntax_element type) { \
        return type == syntax_element::Storage; \
      } \
    };
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

  #define declare_hierarchy(Base) \
    template<typename Syntax> \
    class Base : public Base##_custom_members_t<Base<Syntax>> \
    { \
    public: \
      using syntax = Syntax; \
      using accepts_type = Base##_accepts_type; \
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
      Base(alphabet *sigma, Base##_base *element) \
        : _sigma{sigma}, _element{element} { } \
      \
      template< \
        typename H, \
        REQUIRES( \
          H::hierarchy == hierarchy_type::Base && \
          is_syntax_allowed<typename H::syntax, Syntax> \
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
      type syntax_element() const { return _element->type; } \
      \
      alphabet *sigma() const { return _sigma; }  \
      \
      size_t hash() const { \
        return std::hash<Base##_base const*>{}(_element); \
      } \
      \
      alphabet *_sigma; \
      Base##_base *_element; \
    };

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

  template<
    typename H1, typename H2, 
    REQUIRES(H1::hierarchy == H2::hierarchy)
  >
  bool operator==(H1 h1, H2 h2) {
    return h1._element == h2._element;
  }

  template<
    typename H1, typename H2, 
    REQUIRES(H1::hierarchy == H2::hierarchy)
  >
  bool operator!=(H1 h1, H2 h2) {
    return h1._element != h2._element;
  }

  //
  // internal representation classes for each storage kind
  //
  #define declare_storage_kind(Base, Storage) \
    struct Storage##_data_t;

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_t;

  #include <black/new/internal/formula/hierarchy.hpp>

  enum class storage_type {

    #define declare_storage_kind(Base, Storage) Storage,
    #include <black/new/internal/formula/hierarchy.hpp>

  };
  
  template<hierarchy_type H>
  struct hierarchy_base_type_of_;

  template<hierarchy_type H>
  using hierarchy_base_type_of = typename hierarchy_base_type_of_<H>::type;
  
  #define declare_hierarchy(Base) \
    template<> \
    struct hierarchy_base_type_of_<hierarchy_type::Base> { \
      using type = Base##_base; \
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

  //
  // Helper function to transform a Base argument to its underlying element, if 
  // the argument is a Base, leaving it untouched otherwise
  //
  template<typename T, typename = void>
  struct is_hierarchy_ : std::false_type { };

  template<typename T>
  struct is_hierarchy_<T, std::void_t<decltype(T::hierarchy)>> 
    : std::true_type { };

  template<typename T>
  constexpr bool is_hierarchy = is_hierarchy_<T>::value;
  
  //
  // Handle classes.
  // 
  // The definition of an handle is split in pieces because of the macros.
  // - The Storage##_fields CRTP class declare the fields accessors
  // - The Storage class is the handle
  //
  #include <black/new/internal/formula/hierarchy.hpp>

  template<hierarchy_type Hierarchy>
  struct unique_id_of_ { };

  template<hierarchy_type Hierarchy>
  using unique_id_of = typename unique_id_of_<Hierarchy>::type;

  #define declare_hierarchy(Base) \
    enum class Base##_id : uintptr_t { }; \
    \
    template<> \
    struct unique_id_of_<hierarchy_type::Base> { \
      using type = Base##_id; \
    };

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
      return static_cast<unique_id_of<Derived::hierarchy>>(
        reinterpret_cast<uintptr_t>(self()._element)
      );
    }
   
    template<typename F>
    static std::optional<Derived> from(F f) {
      if constexpr(F::hierarchy != Derived::hierarchy) {
        return {};
      } else {
        using storage_t = storage_base_type_of<Derived::storage>;
        using accepts_type = typename Derived::accepts_type;

        if constexpr(
          !Leaf && 
          !is_syntax_allowed<typename F::syntax, typename Derived::syntax>
        ) return {};

        if(!accepts_type::doesit(f._element->type))
          return {};

        auto obj = static_cast<storage_t *>(f._element);
        return std::optional<Derived>{Derived{f._sigma, obj}};
      }
    }
 
  protected:
    Derived &self() { return static_cast<Derived &>(*this); }
    Derived const&self() const { return static_cast<Derived const&>(*this); }
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
      using accepts_type = Storage##_accepts_type; \
      using syntax = Syntax; \
      static constexpr auto hierarchy = hierarchy_type::Base; \
      static constexpr auto storage = storage_type::Storage; \
      \
      using type = typename Syntax::template type<accepts_type>; \
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
          is_syntax_allowed<typename H::syntax, Syntax> \
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
      class alphabet *_sigma; \
      Storage##_t *_element; \
    };

  #define declare_leaf_storage_kind(Base, Storage) \
    class Storage : \
      public Storage##_fields<Storage>, \
      public storage_common_interface<Storage, true>, \
      public Base##_custom_members_t<Storage> \
    { \
      \
      friend struct Storage##_fields<Storage>; \
    public: \
      using accepts_type = Storage##_accepts_type; \
      using syntax = make_fragment<syntax_element::Storage>; \
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
        is_type_allowed<syntax_element::Element, Syntax>, \
        "'" #Element "' instance not allowed in its own syntax" \
      ); \
    public: \
      using accepts_type = Element##_accepts_type; \
      using syntax = Syntax; \
      static constexpr auto hierarchy = hierarchy_type::Base; \
      static constexpr auto storage = storage_type::Storage; \
      \
      using type = typename Syntax::template type<accepts_type>; \
      \
      Element(Element const&) = default; \
      Element(Element &&) = default; \
      \
      Element(class alphabet *sigma, Storage##_t *element); \
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
    };
  
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    class Element : \
      public Storage##_fields<Element>, \
      public storage_common_interface<Element, true>, \
      public Base##_custom_members_t<Element> \
    { \
      friend struct Storage##_fields<Element>; \
    public: \
      using accepts_type = Element##_accepts_type; \
      using syntax = make_fragment<syntax_element::Element>; \
      static constexpr auto hierarchy = hierarchy_type::Base; \
      static constexpr auto storage = storage_type::Storage; \
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
  #define declare_children(Base, Storage, Hierarchy, Child)  \
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
}

#endif // BLACK_INTERNAL_FORMULA_INTERFACE_HPP
