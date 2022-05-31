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

  //
  // enum type of all the elements of the hierarchies
  //
  #define declare_hierarchy(Base) \
    enum class Base##_type : uint8_t { \

  #define declare_leaf_storage_kind(Base, Storage) Storage,
  #define has_no_hierarchy_elements(Base, Storage) Storage,
  #define declare_hierarchy_element(Base, Storage, Element) Element,

  #define end_hierarchy(Base) \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // Base class for internal representation of elements of the hierarchies
  //
  #define declare_hierarchy(Base) \
  struct Base##_base { \
    Base##_type type; \
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

  
  #define declare_hierarchy(Base) \
    template<typename Syntax> \
    class Base \
    { \
    public: \
      using type = black::internal::new_api::Base##_type; \
      \
      Base() = delete; \
      Base(Base const&) = default; \
      Base(Base &&) = default; \
      \
      Base(alphabet *sigma, Base##_base *element) \
        : _sigma{sigma}, _element{element} { }
      
  #define declare_storage_kind(Base, Storage) \
      Base(Storage<Syntax> const&s);
  
  #define declare_leaf_storage_kind(Base, Storage) \
      Base(Storage const&s);
  
  #define declare_hierarchy_element(Base, Storage, Element) \
      Base(Element<Syntax> const&s);
  
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
      Base(Element const&s);

  #define end_hierarchy(Base) \
      Base &operator=(Base const&) = default; \
      Base &operator=(Base &&) = default; \
      \
      friend bool operator==(Base b1, Base b2) { \
        return b1._element == b2._element; \
      } \
      \
      friend bool operator!=(Base b1, Base b2) { \
        return b1._element != b2._element; \
      } \
      template<typename H> \
      std::optional<H> to() const { \
        black_assert(_element != nullptr); \
        if(!H::accepts_type(_element->type)) \
          return {}; \
        \
        auto obj = static_cast<typename H::storage_t *>(_element); \
        return std::optional<H>{H{_sigma, obj}}; \
      } \
      \
      template<typename H> \
      bool is() const { \
        return to<H>().has_value(); \
      } \
      \
      type Base##_type() const { return _element->type; } \
      \
      alphabet *sigma() const { return _sigma; }  \
      \
      size_t hash() const { \
        return std::hash<Base##_base *>{}(_element); \
      } \
      \
    private: \
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
 
  //
  // constexpr functions to categorize hierarchy type values into storage kinds
  //
  #define declare_storage_kind(Base, Storage) \
  constexpr bool is_##Storage##_type(Base##_type type) { \
    return 

  #define has_no_hierarchy_elements(Base, Storage) \
    type == Base##_type::Storage ||

  #define declare_hierarchy_element(Base, Storage, Element) \
    type == Base##_type::Element ||
    
  #define end_storage_kind(Base, Storage) false; }

  #define declare_leaf_storage_kind(Base, Storage) \
  constexpr bool is_##Storage##_type(Base##_type type) { \
    return type == Base##_type::Storage; \
  }
  #define end_leaf_storage_kind(Base, Storage)

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

  //
  // Enums with list of element types of a certain storage kind
  //
  #define declare_storage_kind(Base, Storage) \
    enum class Storage##_type : uint8_t {
  
  #define declare_no_hierarchy_element(Base, Storage) \
      Storage = to_underlying(Base##_type::Storage),

  #define declare_hierarchy_element(Base, Storage, Element) \
      Element = to_underlying(Base##_type::Element),
  
  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // Helper function to transform a Base argument to its underlying element, if 
  // the argument is a Base, leaving it untouched otherwise
  //
  #define declare_hierarchy(Base) \
    template<typename T> \
    auto Base##_handle_args(T v) { \
      if constexpr(has_element<T>::value) \
        return (Base##_base *)v._element; \
      else \
        return v; \
    }

  #define declare_storage_kind(Base, Storage) \
    inline Base##_type Base##_handle_args(Storage##_type t) { \
      return Base##_type{to_underlying(t)}; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // Handle classes.
  // 
  // The definition of an handle is split in pieces because of the macros.
  // - The Storage##_fields CRTP class declare the fields accessors
  // - The Storage##_type enum lists the types handled by the handle
  // - The Storage class is the handle
  //
  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy(Base) \
    enum class Base##_id : uintptr_t { };

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax, typename H> \
    struct Storage##_fields {

    #define declare_field(Base, Storage, Type, Field) \
      Type Field() const;

    #define declare_child(Base, Storage, Child) \
      Base<Syntax> Child() const;

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    class Storage : public Storage##_fields<Syntax, Storage<Syntax>> { \
      \
      friend struct Storage##_fields<Syntax, Storage<Syntax>>; \
    public: 

  #define declare_leaf_storage_kind(Base, Storage) \
    class Storage : public Storage##_fields<void, Storage> { \
      \
      friend struct Storage##_fields<void, Storage>; \
      using Syntax = syntax<Storage>; \
    public: 

  #define declare_hierarchy_element(Base, Storage, Element) \
        Storage(Element<Syntax> const&e);
  
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
        Storage(Element const&e);

  #define end_storage_kind(Base, Storage) \
      static constexpr auto accepts_type = is_##Storage##_type; \
      \
      using storage_t = Storage##_t; \
      using type = Storage##_type; \
      \
      Storage(Storage const&) = default; \
      Storage(Storage &&) = default; \
      \
      Storage(class alphabet *sigma, Storage##_t *element) \
        : _sigma{sigma}, _element{element} { } \
      \
      template<typename ...Args> \
      Storage(Args ...args); \
      \
      template<typename H> \
      std::optional<H> to() const { \
        return Base<Syntax>{*this}.template to<H>(); \
      } \
      \
      template<typename H> \
      bool is() const { \
        return to<H>().has_value(); \
      } \
      \
      Storage &operator=(Storage const&) = default; \
      Storage &operator=(Storage &&) = default; \
      \
      alphabet *sigma() const { return _sigma; } \
      Base##_id unique_id() const { \
        return static_cast<Base##_id>(reinterpret_cast<uintptr_t>(_element)); \
      } \
      \
      class alphabet *_sigma; \
      Storage##_t *_element; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    class Element : public Storage##_fields<Syntax, Element<Syntax>> { \
      friend struct Storage##_fields<Syntax, Element<Syntax>>; \
    public: \
      static constexpr bool accepts_type(Base##_type t) { \
        return t == Base##_type::Element; \
      } \
      \
      using storage_t = Storage##_t; \
      using type = Storage##_type; \
      \
      Element(Element const&) = default; \
      Element(Element &&) = default; \
      \
      Element(class alphabet *sigma, Storage##_t *element) \
        : _sigma{sigma}, _element{element} { \
          black_assert(_element->type == Base##_type::Element); \
        } \
      \
      template<typename ...Args> \
      Element(Args ...args); \
      \
      Element &operator=(Element const&) = default; \
      Element &operator=(Element &&) = default; \
      \
      alphabet *sigma() const { return _sigma; } \
      \
      Base##_id unique_id() const { \
        return static_cast<Base##_id>(reinterpret_cast<uintptr_t>(_element)); \
      } \
      \
      class alphabet *_sigma; \
      Storage##_t *_element; \
    };
  
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    class Element : public Storage##_fields<void, Element> { \
      friend struct Storage##_fields<void, Element>; \
    public: \
      static constexpr bool accepts_type(Base##_type t) { \
        return t == Base##_type::Element; \
      } \
      \
      using storage_t = Storage##_t; \
      using type = Storage##_type; \
      \
      Element(Element const&) = default; \
      Element(Element &&) = default; \
      \
      Element(class alphabet *sigma, Storage##_t *element) \
        : _sigma{sigma}, _element{element} { \
          black_assert(_element->type == Base##_type::Element); \
        } \
      \
      template<typename ...Args> \
      Element(Args ...args); \
      \
      Element &operator=(Element const&) = default; \
      Element &operator=(Element &&) = default; \
      \
      alphabet *sigma() const { return _sigma; } \
      \
      Base##_id unique_id() const { \
        return static_cast<Base##_id>(reinterpret_cast<uintptr_t>(_element)); \
      } \
      \
      class alphabet *_sigma; \
      Storage##_t *_element; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  //
  // tuple-like access
  //
  #define declare_storage_kind(Base, Storage) \
    constexpr size_t Storage##_arity() { \
      size_t arity = 0;

  #define declare_child(Base, Storage, Child) \
      arity++;

  #define end_storage_kind(Base, Storage) \
      return arity; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_leaf_storage_kind(Base, Storage)

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

  #define declare_leaf_hierarchy_element(Base, Storage, Element)
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
  
  #define declare_child(Base, Storage, Child) Base<Syntax>,
  
  #define end_storage_kind(Base, Storage) void *>;
  #define end_leaf_storage_kind(Base, Storage)

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_leaf_storage_kind(Base, Storage)
  #define declare_storage_kind(Base, Storage) \
    template<int I, typename Syntax, REQUIRES(I < Storage##_arity())> \
    Base<Syntax> get([[maybe_unused]] Storage<Syntax> s) {  \
      return std::get<I>(Storage##_unpack_t<Syntax>{

  #define declare_child(Base, Storage, Child)  \
        s.Child(),
  
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
