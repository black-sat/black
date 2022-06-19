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

#include <memory>

//
// This file expands the macros used in `hierarchy.hpp` to generate the actual
// types exposed by BLACK's logic API. Before reading here, be sure to have
// understood the documentation at the top of `hierarchy.hpp`. The declarations
// are for the most part specializations or derived classes of entities declared
// in `core.hpp`, so read the comments in that file as well before reading 
// here. 
//
// This file is a long sequence of blocks that start with the declaration of
// some macros (e.g. `declare_hierarchy`) and end with the inclusion of
// `hierarchy.hpp`. Since `hierarchy.hpp` undefines all of its macros at the
// end, after each inclusion the preprocessing environment is clean again and
// ready for the next inclusion. The order of the declarations try to follow
// that of `core.hpp`, excepting when forbidden by interdependencies between
// declared entities.
//


namespace black::internal::logic
{
  class alphabet;

  //
  // Here we declare the three enumerations that drive all the system.
  //
  // `hierarchy_type` lists all the hierarchy type as per `declare_hierarchy`
  enum class hierarchy_type  : uint8_t {
    #define declare_hierarchy(Base) Base,
    #include <black/internal/logic/hierarchy.hpp>
  };

  // here we have all the storage kinds
  enum class storage_type  : uint8_t {
    #define declare_storage_kind(Base, Storage) Storage,
    #include <black/internal/logic/hierarchy.hpp>
  };

  // this is a bit more complex. Here we have to list all the hierarchy
  // elements, but also storage kinds that have no hierarchy elements, so leaf
  // storage kinds, and those marked with `has_no_hierarchy_elements`.
  enum class syntax_element : uint8_t {
    #define declare_leaf_storage_kind(Base, Storage) Storage,
    #define has_no_hierarchy_elements(Base, Storage) Storage,
    #define declare_hierarchy_element(Base, Storage, Element) Element,

    #include <black/internal/logic/hierarchy.hpp>
  };

  //
  // Printing of `syntax_element` values. This is just for debugging.
  //
  inline std::string to_string(syntax_element e) {
    switch(e) {

    #define declare_leaf_storage_kind(Base, Storage) \
      case syntax_element::Storage: return #Storage;
    #define has_no_hierarchy_elements(Base, Storage) \
      case syntax_element::Storage: return #Storage;
    #define declare_hierarchy_element(Base, Storage, Element) \
      case syntax_element::Element: return #Element;

    #include <black/internal/logic/hierarchy.hpp>

    }
  }

  // Specializations of the trait to associate to each storage kind its
  // hierarchy
  #define declare_storage_kind(Base, Storage) \
    template<> \
    struct hierarchy_of_storage<storage_type::Storage> { \
      static constexpr auto value = hierarchy_type::Base; \
    };
  
  #include <black/internal/logic/hierarchy.hpp>
  
  // Specializations of the trait to associate to each syntax element its
  // storage kind. We have to do the same thing for all the entities included in
  // the declaration of `syntax_element`, so hierarchy elements, leaf storage
  // kinds, and those with `has_no_hierarchy_elements`.
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
  
  #include <black/internal/logic/hierarchy.hpp>

  #undef declare_storage_of_element

  // Specializations of the trait to associate a `syntax_element` to those
  // storage kinds which correspond to a single `syntax_element`, such as leaf
  // storage kinds and those with `has_no_hierrchy_elements`.
  #define declare_leaf_storage_kind(Base, Storage) \
  template<> \
  struct element_of_storage<storage_type::Storage> { \
    static constexpr auto value = syntax_element::Storage; \
  };

  #define has_no_hierarchy_elements(Base, Storage) \
    declare_leaf_storage_kind(Base, Storage)

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Custom syntax predicate for each hierarchy type, enumerating all its syntax
  // elements.
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

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Same thing for the syntax predicate associated with storage kinds.
  //
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
  
  #include <black/internal/logic/hierarchy.hpp>

  //
  // Specialization of `fragment_enum_element` for each `syntax_element`. Each
  // specialization provides a static member of type
  // `type_value<Element>` named exactly `Element`. The result is that,
  // e.g. `unary<LTL>::type` has members such as `unary<LTL>::type::always` and
  // `unary<LTL>::type::eventually`.
  //
  #define declare_enum_element(Element) \
    template<> \
    struct fragment_enum_element<syntax_element::Element> { \
      using Element = type_value<syntax_element::Element>; \
    };

  #define declare_leaf_storage_kind(Base, Storage) declare_enum_element(Storage)
  #define has_no_hierarchy_elements(Base, Storage) declare_enum_element(Storage)
  #define declare_hierarchy_element(Base, Storage, Element) \
    declare_enum_element(Element)

  #include <black/internal/logic/hierarchy.hpp>

  #undef declare_enum_element

  //
  // Declaration of `universal_fragment_t`, one fragment to rule them all.
  //
  struct universal_fragment_t \
    : make_fragment_cpp_t<0  

  #define declare_leaf_storage_kind(Base, Storage) , syntax_element::Storage
  #define has_no_hierarchy_elements(Base, Storage) , syntax_element::Storage
  #define declare_hierarchy_element(Base, Storage, Element) \
    , syntax_element::Element
  #include <black/internal/logic/hierarchy.hpp>

    > { };

  //
  // Now we need to forward declare the storage and hierarchy element types that
  // will be declared later. We need the forward declarations now because
  // `storage_data` below may refer to a specific type (e.g. the field of type
  // `variable` of `quantifier`).
  //
  #define declare_hierarchy(Base) \
    template<fragment Syntax> \
    struct Base;
  #define declare_hierarchy_element(Base, Storage, Element) \
    template<fragment Syntax> \
    class Element;
  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    class Storage;
  #define declare_simple_hierarchy(Base) \
    struct Base;
  #define declare_simple_storage_kind(Base, Storage) \
    class Storage;
  #define declare_leaf_storage_kind(Base, Storage) \
    class Storage;
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    class Element;

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Specializations of `storage_data`. Here we define the internal layout of
  // all the fields, children and children vectors of a storage kind.
  //
  #define declare_storage_kind(Base, Storage) \
    template<> \
    struct storage_data<storage_type::Storage> \
      : make_storage_data_cpp<0

  #define declare_field(Base, Storage, Type, Field) , Type

  #define declare_child(Base, Storage, Hierarchy, Child) \
    , hierarchy_node<hierarchy_type::Hierarchy> const *
  
  #define declare_children(Base, Storage, Hierarchy, Children) \
    , std::vector<hierarchy_node<hierarchy_type::Hierarchy> const*>

  #define end_storage_kind(Base, Storage) \
      > { };

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Now we can declare the concrete hierarchy types. They derive from
  // `hierarchy_base` and from `hierachy_custom_members` and do nothing else
  // than re-exporting `hierarchy_base`'s constructors. At the same time, we
  // declare a deduction guide to use the hierarchy type without template
  // parameters, and the concept `is_<hierarchy>`.
  //
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
    template<typename T> \
    concept is_##Base = hierarchy<T> && T::hierarchy == hierarchy_type::Base;

  #define declare_simple_hierarchy(Base) \
    struct Base \
      : hierarchy_base<hierarchy_type::Base, universal_fragment_t>, \
        hierarchy_custom_members<hierarchy_type::Base, Base> \
    { \
      using hierarchy_base< \
        hierarchy_type::Base, universal_fragment_t \
      >::hierarchy_base; \
    }; \
    \
    template<typename T> \
    concept is_##Base = hierarchy<T> && T::hierarchy == hierarchy_type::Base;

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we specialize the trait `hierarchy_type_of`, which returns the
  // concrete hierarchy type given a value of the `hierarchy_type` enum and a
  // fragment.
  //
  #define declare_hierarchy(Base) \
    template<fragment Syntax> \
    struct hierarchy_type_of<Syntax, hierarchy_type::Base> { \
      using type = Base<Syntax>; \
    };
  
  #define declare_simple_hierarchy(Base) \
    template<fragment Syntax> \
    struct hierarchy_type_of<Syntax, hierarchy_type::Base> { \
      using type = Base; \
    };

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we specialize `hierarchy_has_standard_equality`. Hierarchies that have
  // the `has_no_standard_equality` macro (currently only `term`), here answer
  // `false`.
  //
  #define has_no_standard_equality(Base) \
    template<> \
    struct hierarchy_has_standard_equality<hierarchy_type::Base> \
      : std::false_type { };

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we start to talk about storage kinds. This traits tells whether a
  // storage kind has hierarchy elements.
  //
  #define has_no_hierarchy_elements(Base, Storage) \
    template<> \
    struct storage_has_hierarchy_elements<storage_type::Storage> \
      : std::false_type { };

  #define declare_leaf_storage_kind(Base, Storage) \
    template<> \
    struct storage_has_hierarchy_elements<storage_type::Storage> \
      : std::false_type { };

  #include <black/internal/logic/hierarchy.hpp>

  //
  // This is the base class of `storage_base` that exposes the member functions
  // named after the fields of the storage kind. Here we only declare the
  // accessor functions, which are defined later.
  //
  #define declare_storage_kind(Base, Storage) \
    template<typename Derived> \
    struct storage_fields_base<storage_type::Storage, Derived> {

    #define declare_field(Base, Storage, Type, Field) \
      Type Field() const;

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/internal/logic/hierarchy.hpp>

  //
  // And this is the base class exposing the member functions named after the
  // children and children vectors of the storage kind. Here we only declare the
  // accessor functions, which are defined later.
  //
  #define declare_storage_kind(Base, Storage) \
    template<typename Derived, fragment Syntax> \
    struct storage_children_base<storage_type::Storage, Syntax, Derived> {

    #define declare_child(Base, Storage, Hierarchy, Child) \
      Hierarchy<Syntax> Child() const;

    #define declare_children(Base, Storage, Hierarchy, Children) \
      auto Children() const;

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we derived from the custom members type if specified by the
  // `declare_storage_custom_members` macros.
  //
  #define declare_storage_custom_members(Base, Storage, Struct) \
    template<typename Derived> \
    struct storage_custom_members<storage_type::Storage, Derived> \
      : Struct<Derived> { };

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we define storage kinds. These are only non-leaf storage kinds, which
  // thus are templated over the fragment of their children. We inherit from
  // `storage_base` and we inherit its constructors, but we also need to declare
  // the "allocating constructor", i.e. the one that allocates new storage kinds
  // given the concrete type and their children. These constructors have to be
  // declared after the `alphabet_base` class defined later, because they call
  // into it, so here are only declared. After the class we also declare a
  // deduction guide for the converting constructor.
  //
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
  
  #define declare_simple_storage_kind(Base, Storage) \
    class Storage : \
      public \
        storage_base<storage_type::Storage, universal_fragment_t, Storage> \
    { \
      using base_t = \
        storage_base<storage_type::Storage, universal_fragment_t, Storage>; \
    public: \
      using base_t::base_t; \
      \
      template<typename ...Args> \
        requires is_storage_constructible_v< \
          storage_type::Storage, universal_fragment_t, Args... \
        > \
      explicit Storage(Args ...args); \
    };

  //
  // This is a deduction guide for use with the allocating constructor. This is
  // only declared for storage kinds without hierarchy elements (e.g. `atom<>`)
  // since in case of hierarchy elements, in ordert o pass the `type` parameter
  // one would need to spell the fragment anyway (e.g.
  // `unary<LTL>(unary<LTL>::type::always, p, q)`).
  //
  #define has_no_hierarchy_elements(Base, Storage) \
    template<typename ...Args> \
    explicit Storage(Args ...args) -> \
      Storage<deduce_fragment_for_storage_t<syntax_element::Storage, Args...>>;

  //
  // Here we declare the concrete type for leaf storage kinds. This is easier
  // because we do not have the allocating constructor (leaf storage kinds are
  // allocated by `alphabet`, e.g. `sigma.boolean()`). Moreover, leaf storage
  // kinds have a precise `syntax_element`, so we inherit from
  // `hierarchy_element_base`.
  //
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

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we specialize the `storage_type_of` trait which returns the concrete
  // type for a given `storage_type` and a given fragment.
  //
  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    struct storage_type_of<Syntax, storage_type::Storage> { \
      using type = Storage<Syntax>; \
    };
  
  #define declare_simple_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    struct storage_type_of<Syntax, storage_type::Storage> { \
      using type = Storage; \
    };

  #define declare_leaf_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    struct storage_type_of<Syntax, storage_type::Storage> { \
      using type = Storage; \
    };

  #include <black/internal/logic/hierarchy.hpp>  


  //
  // Here we declare concrete types for hierarchy elements. Again, the non-leaf
  // ones are templated over the fragment, while the leaf ones are not. The
  // non-leaf ones have the same allocating constructor as non-leaf storage
  // kinds, defined later as well. We also declare the usual deduction guide.
  //
  #define declare_hierarchy_element(Base, Storage, Element) \
    template<fragment Syntax> \
    class Element : \
      public hierarchy_element_base< \
        syntax_element::Element, Syntax, Element<Syntax> \
      > \
    { \
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
  
  //
  // Concrete type for leaf hierarchy elements. This is just a very thin class
  // derived from `hierarchy_element_base`.
  //
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

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Trait to obtain the concrete type given a `syntax_element`. This works for
  // hierarchy elements but also for leaf storage kinds and storage kinds
  // without hierarchy elements, which all have their specific `syntax_element`.
  //
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

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we start talking about the alphabet and the allocation of storage
  // kinds. This is the type that defines which arguments are accepted by the
  // allocating constructor of storage kinds. Fields are accepted as-is, while
  // children and children vectors are accepted through wrappers that later can
  // be implicitly converted to their underlying type (i.e. a `formula<Syntax>`
  // argument becomes `hierarchy_node_t<hierarchy_type::formula> const*`).
  //
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

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here it finally comes the `alphabet_base` class, which `alphabet` will
  // inherit without adding too much (see `interface.hpp`). The class is default
  // constructible and movable, but not copyable. We generate a lot of members,
  // see later for each inclusion.
  //
  class alphabet_base
  {
  public:
    alphabet_base();
    ~alphabet_base();

    alphabet_base(alphabet_base const&) = delete;
    alphabet_base(alphabet_base &&);

    alphabet_base &operator=(alphabet_base const&) = delete;
    alphabet_base &operator=(alphabet_base &&);

    //
    // Here we generate the allocation functions for leaf storage kinds, e.g.
    // `sigma.boolean()` or `sigma.proposition()`. The function calls
    // `unique_<storage>()` (e.g. `unique_boolean()`), which is declared later
    // and defined in `logic.cpp`. To that, we pass a temporary node object
    // allocated directly from the function arguments (since there are no
    // children, we do not have to wrap anything). The `unique_<storage>()`
    // function will take this object and return the address of a uniqued copy
    // of it. We also generate an overload of the `element()` member function,
    // which does the same thing but takes which element to allocate as a
    // template parameter of type `syntax_element`.
    //
    #define declare_leaf_storage_kind(Base, Storage) \
      template<typename ...Args> \
        requires is_leaf_storage_constructible_v<Storage, Args...> \
      class Storage Storage(Args ...args) { \
        return \
          ::black::internal::logic::Storage{ \
            this, \
            unique_##Storage( \
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

    //
    // Same thing as above, for leaf hierarchy elements.
    //
    #define declare_leaf_hierarchy_element(Base, Storage, Element) \
      template<typename ...Args> \
        requires is_leaf_storage_constructible_v<Element, Args...> \
      class Element Element(Args ...args) { \
        return \
          ::black::internal::logic::Element{ \
            this, \
            unique_##Storage( \
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

    #include <black/internal/logic/hierarchy.hpp>

    //
    // Here we declare as friends all the non-leaf storage and hierarchy element
    // types, because their allocating constructors have to call the private
    // `unique_<storage>` members.
    //
    #define declare_leaf_storage_kind(Base, Storage)
    #define declare_storage_kind(Base, Storage) \
      template<fragment Syntax> \
      friend class Storage;
    
    #define declare_simple_storage_kind(Base, Storage) \
      friend class Storage;
    
    #define declare_leaf_hierarchy_element(Base, Storage, Element)
    #define declare_hierarchy_element(Base, Storage, Element) \
      template<fragment Syntax> \
      friend class Element;

    #include <black/internal/logic/hierarchy.hpp>

  private:
    //
    // These member functions are defined out-of-line in `logic.cpp`.
    //
    #define declare_storage_kind(Base, Storage) \
      storage_node<storage_type::Storage> *unique_##Storage( \
        storage_node<storage_type::Storage> node \
      );

    #include <black/internal/logic/hierarchy.hpp>

    // pimpl pointer to `alphabet_impl`, defined in `logic.cpp`.
    struct alphabet_impl;
    std::unique_ptr<alphabet_impl> _impl;
  };  

  //
  // Now that we have the `alphabet_base` class, we can define the allocating
  // constructors declared above of storage kinds and hierarchy elements. We get
  // a pointer to the aphabet by one of the arguments using `get_sigma()`. Then,
  // we construct an object of type `storage_alloc_args_t<>` from the arguments
  // of the function, which is passed to `args_to_node` which creates a
  // temporary node object from those arguments (with the due unwrapping, e.g.
  // from `formula<Syntax>` to `hierarchy_node<hierarchy_type::formula>
  // const*`). Then, the `unique_<storage>` member function of `alphabet_base`
  // (e.g. `unique_boolean`) is used to get the address of the uniqued copy of
  // the node.
  //
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
          get_sigma(args...)->unique_##Storage( \
            args_to_node<Syntax, storage_type::Storage>( \
              storage_alloc_args_t<Syntax, storage_type::Storage>{args...} \
            ) \
          ) \
        } { }
  
  #define declare_simple_storage_kind(Base, Storage) \
    template<typename ...Args> \
      requires is_storage_constructible_v< \
        storage_type::Storage, universal_fragment_t, Args... \
      > \
    Storage::Storage(Args ...args) \
      : Storage{ \
          get_sigma(args...), \
          get_sigma(args...)->unique_##Storage( \
            args_to_node<universal_fragment_t, storage_type::Storage>( \
              storage_alloc_args_t< \
                universal_fragment_t, storage_type::Storage \
              >{args...} \
            ) \
          ) \
        } { }

  //
  // Same thing as above for hierarchy elements.
  //
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
          get_sigma(args...)->unique_##Storage( \
            args_to_node<Syntax, storage_type::Storage>( \
              storage_alloc_args_t<Syntax, storage_type::Storage>{ \
                typename Storage<Syntax>::type::Element{}, \
                args... \
              } \
            ) \
          ) \
        } { }

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we start to account for the other elements of the interface of
  // hierarchy types. See especially the comments for the `index_of_field` trait
  // in `core.hpp`. Here we declare a numbero f of `const char *` constants, one
  // for each possible field/child/children vector of all the storage kinds of
  // all the hierarchies.
  //
  #define declare_field(Base, Storage, Type, Field) \
    inline constexpr const char Storage##_##Field##_field[] = #Field;

  #define declare_child(Base, Storage, Hierarchy, Child) \
    declare_field(Base, Storage, Hierarchy, Child)
  #define declare_children(Base, Storage, Hierarchy, Children) \
    declare_field(Base, Storage, Hierarchy, Children)

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we generate an array of string literals that maps the index of a
  // field/child/child vector to its name (as a string). The last literal of the
  // array is there to avoid declaring an empty array, for storage kinds without
  // fields nor children (e.g. `primitive_sort`).
  //
  #define declare_storage_kind(Base, Storage) \
    inline constexpr std::string_view Storage##_fields[] = {
  
  #define declare_field(Base, Storage, Type, Field) #Field, 
  #define declare_child(Base, Storage, Hierarchy, Child) #Child, 
  #define declare_children(Base, Storage, Hierarchy, Children) #Children,

  #define end_storage_kind(Base, Storage) \
      "__THIS_SHOULD_NOT_SHOW_UP_ANYWHERE__" \
    };

  #include <black/internal/logic/hierarchy.hpp>

  //
  // This trait returns the declare hierarchy type of a children or children
  // vector. 
  //
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
  
  #include <black/internal/logic/hierarchy.hpp>

  //
  // Here we define the actual accessor member functions of
  // `storage_fields_base` and `storage_children_base` declared above. Here we
  // have the name of the field, but we need the index to get it from
  // `storage_data_t` which is a tuple, so we map names to indexes using the
  // arrays of string literals defined above.
  //
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
    auto \
    storage_children_base<storage_type::Storage, Syntax, H>::Children() const {\
      constexpr size_t I = \
        index_of_field_v<Storage##_fields, Storage##_##Children##_field>;\
      return get_children<I, Syntax>(static_cast<H const&>(*this)); \
    }

  #include <black/internal/logic/hierarchy.hpp>

  //
  // Lastly, two traits used by the get<>() function to know whether a given
  // index refer to a field, a child, or a children vector.
  //
  #define declare_field(Base, Storage, Type, Field) \
    template<> \
    struct storage_ith_data_is_field< \
      index_of_field_v<Storage##_fields, Storage##_##Field##_field>, \
      storage_type::Storage \
    > : std::true_type { };
  
  #define declare_child(Base, Storage, Hierarchy, Child) \
    template<> \
    struct storage_ith_data_is_child< \
      index_of_field_v<Storage##_fields, Storage##_##Child##_field>, \
      storage_type::Storage \
    > : std::true_type { };

  #include <black/internal/logic/hierarchy.hpp>

}

#endif // BLACK_INTERNAL_FORMULA_INTERFACE_HPP
