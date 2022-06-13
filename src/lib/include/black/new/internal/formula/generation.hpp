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

  struct universal_fragment_t \
    : make_fragment_cpp_t<0  

  #define declare_leaf_storage_kind(Base, Storage) , syntax_element::Storage
  #define has_no_hierarchy_elements(Base, Storage) , syntax_element::Storage
  #define declare_hierarchy_element(Base, Storage, Element) \
    , syntax_element::Element
  #include <black/new/internal/formula/hierarchy.hpp>

    > { };


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
    template<fragment Syntax> \
    struct element_type_of<Syntax, syntax_element::Element> { \
      using type = Element<Syntax>; \
    };
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    template<fragment Syntax> \
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
      : make_syntax_predicate<syntax_list<syntax_element::Storage>> { };

  #define end_leaf_storage_kind(Base, Storage)
  
  #include <black/new/internal/formula/hierarchy.hpp>
  
  #define declare_hierarchy(Base) \
    template<fragment Syntax> \
    struct Base \
      : hierarchy_base<hierarchy_type::Base, Syntax>, \
        hierarchy_custom_members<hierarchy_type::Base, Base<Syntax>> \
    { \
      using hierarchy_base<hierarchy_type::Base, Syntax>::hierarchy_base; \
    }; \
    \
    template<typename H> \
    Base(H const&) -> Base<typename H::syntax>; \
  \
  template<fragment Syntax> \
  struct concrete_hierarchy_type<hierarchy_type::Base, Syntax> { \
    using type = Base<Syntax>; \
  }; \
  \
  template<typename T> \
  concept is_##Base = hierarchy<T> && T::hierarchy == hierarchy_type::Base;

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

  #define declare_storage_custom_members(Base, Storage, Struct) \
    template<typename Derived> \
    struct storage_custom_members<storage_type::Storage, Derived> \
      : Struct<Derived> { };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    class Storage : \
      public \
        storage_base<storage_type::Storage, Syntax, Storage<Syntax>> \
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
    Storage(H const&) -> Storage<typename H::syntax>;

  #define has_no_hierarchy_elements(Base, Storage) \
    template<typename ...Args> \
    explicit Storage(Args ...args) -> \
      Storage<deduce_fragment_for_storage_t<syntax_element::Storage, Args...>>;

  #define declare_leaf_storage_kind(Base, Storage) \
    class Storage : \
      public hierarchy_element_base< \
        syntax_element::Storage, make_fragment_t<syntax_element::Storage>, \
        Storage \
      > \
    { \
    public: \
      using hierarchy_element_base< \
        syntax_element::Storage, make_fragment_t<syntax_element::Storage>, \
        Storage \
      >::hierarchy_element_base; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  
  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    struct concrete_storage_type<storage_type::Storage, Syntax> { \
      using type = Storage<Syntax>; \
    };

  #define declare_leaf_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    struct concrete_storage_type<storage_type::Storage, Syntax> { \
      using type = Storage; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<fragment Syntax> \
    class Element : \
      public hierarchy_element_base< \
        syntax_element::Element, Syntax, Element<Syntax> \
      > \
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
      Element<deduce_fragment_for_storage_t<syntax_element::Element, Args...>>;
  
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    class Element : \
      public hierarchy_element_base< \
        syntax_element::Element, make_fragment_t<syntax_element::Element>, \
        Element \
      > \
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
    template<> \
    struct storage_arity<storage_type::Storage> \
      : std::integral_constant<size_t, 

  #define declare_child(Base, Storage, Hierarchy, Child) 1 +

  #define declare_children(Base, Storage, Hierarchy, Children) 1 +

  #define end_storage_kind(Base, Storage) \
      0> { };

  #include <black/new/internal/formula/hierarchy.hpp>


  #define declare_children(Base, Storage, Hierarchy, Children) \
    template<> \
    struct storage_has_children_vector<storage_type::Storage> \
      : std::true_type { };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    struct storage_alloc_args<Syntax, storage_type::Storage> \
      : make_storage_alloc_args<Syntax, storage_type::Storage \
  
  #define declare_field(Base, Storage, Type, Field) , Type

  #define declare_child(Base, Storage, Hierarchy, Child) \
    , child_wrapper<hierarchy_type::Hierarchy, Syntax>

  #define declare_children(Base, Storage, Hierarchy, Children) \
    , children_wrapper<hierarchy_type::Hierarchy, Syntax>

  #define end_storage_kind(Base, Storage) \
      > { };

  #include <black/new/internal/formula/hierarchy.hpp>

  class alphabet_base
  {
  public:
    alphabet_base();
    ~alphabet_base();

    alphabet_base(alphabet_base const&) = delete;
    alphabet_base(alphabet_base &&);

    alphabet_base &operator=(alphabet_base const&) = delete;
    alphabet_base &operator=(alphabet_base &&);

    #define declare_leaf_storage_kind(Base, Storage) \
      template<typename ...Args> \
        requires is_leaf_storage_constructible_v<Storage, Args...> \
      class Storage Storage(Args ...args) { \
        return \
          ::black::internal::new_api::Storage{ \
            this, \
            allocate_##Storage( \
              storage_node<storage_type::Storage>{ \
                syntax_element::Storage, args... \
              } \
            ) \
          }; \
      } \
      \
      template<syntax_element Element, typename ...Args> \
        requires (Element == syntax_element::Storage) \
      auto element(Args ...args) -> decltype(Storage(args...)) { \
        return Storage(args...); \
      }

    #define declare_leaf_hierarchy_element(Base, Storage, Element) \
      template<typename ...Args> \
        requires is_leaf_storage_constructible_v<Element, Args...> \
      class Element Element(Args ...args) { \
        return \
          ::black::internal::new_api::Element{ \
            this, \
            allocate_##Storage( \
              storage_node<storage_type::Storage>{ \
                syntax_element::Element, args... \
              } \
            ) \
          }; \
      } \
      \
      template<syntax_element E, typename ...Args> \
        requires (E == syntax_element::Element) \
      auto element(Args ...args) -> decltype(Element(args...)) { \
        return Element(args...); \
      }

    #include <black/new/internal/formula/hierarchy.hpp>

    #define declare_storage_kind(Base, Storage) \
      template<fragment Syntax> \
      friend class Storage;
    #define declare_leaf_storage_kind(Base, Storage) \
      friend class Storage;
    #define declare_hierarchy_element(Base, Storage, Element) \
      template<fragment Syntax> \
      friend class Element;
    #define declare_leaf_hierarchy_element(Base, Storage, Element) \
      friend class Element;
    #include <black/new/internal/formula/hierarchy.hpp>

  private:
    #define declare_storage_kind(Base, Storage) \
      storage_node<storage_type::Storage> *allocate_##Storage( \
        storage_node<storage_type::Storage> node \
      );

    #include <black/new/internal/formula/hierarchy.hpp>

    struct alphabet_impl;
    std::unique_ptr<alphabet_impl> _impl;
  };  

  #define declare_field(Base, Storage, Type, Field) \
    inline constexpr const char Storage##_##Field##_field[] = #Field;

  #define declare_child(Base, Storage, Hierarchy, Child) \
    declare_field(Base, Storage, Hierarchy, Child)
  #define declare_children(Base, Storage, Hierarchy, Children) \
    declare_field(Base, Storage, Hierarchy, Children)

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    inline constexpr std::string_view Storage##_fields[] = {
  
  #define declare_field(Base, Storage, Type, Field) #Field, 
  #define declare_child(Base, Storage, Hierarchy, Child) #Child, 
  #define declare_children(Base, Storage, Hierarchy, Children) #Children,

  #define end_storage_kind(Base, Storage) \
      "LAST" \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_child(Base, Storage, Hierarchy, Child) \
  template<> \
  struct hierarchy_of_storage_child< \
    index_of_field_v<Storage##_fields, Storage##_##Child##_field>, \
    storage_type::Storage \
  > { \
    static constexpr auto value = hierarchy_type::Hierarchy; \
  };

  #define declare_children(Base, Storage, Hierarchy, Children) \
    declare_child(Base, Storage, Hierarchy, Children)
  
  #include <black/new/internal/formula/hierarchy.hpp>


  #define declare_field(Base, Storage, Type, Field) \
    template<typename H> \
    Type storage_fields_base<storage_type::Storage, H>::Field() const { \
      constexpr size_t I = \
        index_of_field_v<Storage##_fields, Storage##_##Field##_field>; \
      return get_field<I>(static_cast<H const&>(*this)); \
    }

  #define declare_child(Base, Storage, Hierarchy, Child) \
    template<typename H, fragment Syntax> \
    Hierarchy<Syntax> \
    storage_children_base<storage_type::Storage, Syntax, H>::Child() const { \
      constexpr size_t I = \
        index_of_field_v<Storage##_fields, Storage##_##Child##_field>;\
      return get_child<I, Syntax>(static_cast<H const&>(*this)); \
    }

  #define declare_children(Base, Storage, Hierarchy, Children) \
    template<typename H, fragment Syntax> \
    std::vector<Hierarchy<Syntax>> \
    storage_children_base<storage_type::Storage, Syntax, H>::Children() const {\
      constexpr size_t I = \
        index_of_field_v<Storage##_fields, Storage##_##Children##_field>;\
      return get_children<I, Syntax>(static_cast<H const&>(*this)); \
    }

  #include <black/new/internal/formula/hierarchy.hpp>


  #define declare_leaf_storage_kind(Base, Storage)
  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    template<typename ...Args> \
      requires is_storage_constructible_v< \
        storage_type::Storage, Syntax, Args... \
      > \
    Storage<Syntax>::Storage(Args ...args) \
      : Storage{ \
          get_sigma(args...), \
          get_sigma(args...)->allocate_##Storage( \
            args_to_node<Syntax, storage_type::Storage>( \
              storage_alloc_args_t<Syntax, storage_type::Storage>{args...} \
            ) \
          ) \
        } { }

  #define declare_leaf_hierarchy_element(Base, Storage, Element)
  #define declare_hierarchy_element(Base, Storage, Element) \
    template<fragment Syntax> \
    template<typename ...Args> \
      requires is_hierarchy_element_constructible_v< \
        syntax_element::Element, Syntax, Args... \
      > \
    Element<Syntax>::Element(Args ...args) \
      : Element{ \
          get_sigma(args...), \
          get_sigma(args...)->allocate_##Storage( \
            args_to_node<Syntax, storage_type::Storage>( \
              storage_alloc_args_t<Syntax, storage_type::Storage>{ \
                Storage<Syntax>::type::Element, \
                args... \
              } \
            ) \
          ) \
        } { }

  #include <black/new/internal/formula/hierarchy.hpp>
}

#endif // BLACK_INTERNAL_FORMULA_INTERFACE_HPP
