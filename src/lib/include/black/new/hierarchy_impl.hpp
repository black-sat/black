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

#include <black/support/assert.hpp>
#include <black/support/hash.hpp>

#include <tsl/hopscotch_map.h>

#include <cstdint>
#include <deque>

//
// Note: no include guards. This file is designed to be included many times.
//

namespace black::internal::new_api 
{
  class alphabet;

  //
  // enum type of all the elements of the hierarchies
  //
  #define declare_hierarchy(Base) \
    enum class Base##_type : uint8_t { \

  #define declare_leaf_storage_kind(Base, Storage) Storage,
  #define declare_hierarchy_element(Base, Storage, Element) Element,

  #define end_hierarchy(Base) \
    };

  #include <black/new/hierarchy.hpp>

  //
  // Base class for internal representation of elements of the hierarchies
  //
  #define declare_hierarchy(Base) \
  struct Base##_base { \
    Base##_type type; \
  };

  #include <black/new/hierarchy.hpp>

  #define declare_hierarchy(Base) \
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
        : _sigma{sigma}, _element{element} { } \
      \
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

  #include <black/new/hierarchy.hpp>

  #define declare_hierarchy(Base) \
    } namespace std { \
      template<>                                                 \
      struct hash<black::internal::new_api::Base> {                       \
        size_t operator()(black::internal::new_api::Base const&t) const { \
          return black::internal::new_api::Base{t}.hash();                \
        }                                                        \
      }; \
    } namespace black::internal::new_api {

  #include <black/new/hierarchy.hpp>
 
  //
  // constexpr functions to categorize hierarchy type values into storage kinds
  //
  #define declare_leaf_storage_kind(Base, Storage) \
  constexpr bool is_##Storage##_type(Base##_type type) { \
    return type == Base##_type::Storage; \
  }
  #define end_leaf_storage_kind(Base, Storage)

  #define declare_storage_kind(Base, Storage) \
  constexpr bool is_##Storage##_type([[maybe_unused]] Base##_type type) { \
    return 

  #define declare_hierarchy_element(Base, Storage, Element) \
    type == Base##_type::Element ||
    
  #define end_storage_kind(Base, Storage) false; }

  #include <black/new/hierarchy.hpp>

  //
  // internal representation classes for each storage kind
  //
  #define declare_storage_kind(Base, Storage) \
    struct Storage##_data_t {

  #define declare_field(Base, Storage, Type, Field) \
    Type Field;

  #define declare_child(Base, Storage, Child) \
    Base##_base *Child;

  #define end_storage_kind(Base, Storage)  \
    };

  #include <black/new/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_t : Base##_base { \
      \
      Storage##_t(Base##_type t, Storage##_data_t _data) \
        : Base##_base{t}, data{_data} { \
          black_assert(is_##Storage##_type(t)); \
        } \
      \
      Storage##_data_t data; \
    };

  #define declare_leaf_storage_kind(Base, Storage) \
    struct Storage##_t : Base##_base { \
      static constexpr auto accepts_type = is_##Storage##_type; \
      \
      Storage##_t(Storage##_data_t _data) \
        : Base##_base{Base##_type::Storage}, data{_data} { } \
      \
      Storage##_data_t data; \
    };

  #include <black/new/hierarchy.hpp>

  //
  // Enums with list of element types of a certain storage kind
  //
  #define declare_leaf_storage_kind(Base, Storage) \
    enum class Storage##_type : uint8_t { \
      Storage = to_underlying(Base##_type::Storage) \
    };
  #define end_leaf_storage_kind(Base, Storage)

  #define declare_storage_kind(Base, Storage) \
    enum class Storage##_type : uint8_t {
  
  #define declare_hierarchy_element(Base, Storage, Element) \
      Element = to_underlying(Base##_type::Element),
  
  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/hierarchy.hpp>

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

  #include <black/new/hierarchy.hpp>

  //
  // Handle classes.
  // 
  // The definition of an handle is split in pieces because of the macros.
  // - The Storage##_fields CRTP class declare the fields accessors
  // - The Storage##_type enum lists the types handled by the handle
  // - The Storage class is the handle
  //
  #define declare_hierarchy(Base) \
    enum class Base##_id : uintptr_t { };

  #define declare_storage_kind(Base, Storage) \
    template<typename H> \
    struct Storage##_fields {

    #define declare_field(Base, Storage, Type, Field) \
      Type Field() const { \
        return static_cast<H const&>(*this)._element->data.Field; \
      }

    #define declare_child(Base, Storage, Child) \
      Base Child() const { \
        return Base{ \
          static_cast<H const&>(*this)._sigma,  \
          static_cast<H const&>(*this)._element->data.Child \
        }; \
      }

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    class Storage : public Storage##_fields<Storage> { \
      \
      friend struct Storage##_fields<Storage>; \
    public: \
      static constexpr auto accepts_type = is_##Storage##_type; \
      \
      using storage_t = Storage##_t; \
      using type = Storage##_type; \
      \
      Storage(class alphabet *sigma, Storage##_t *element) \
        : _sigma{sigma}, _element{element} { } \
      \
      template<typename ...Args> \
      Storage(Args ...args); \
      \
      alphabet *sigma() const { return _sigma; } \
      Base##_id unique_id() const { \
        return static_cast<Base##_id>(reinterpret_cast<uintptr_t>(_element)); \
      } \
      \
      operator Base() const { \
        return Base{_sigma, _element}; \
      } \
      \
      class alphabet *_sigma; \
      Storage##_t *_element; \
    };

  #include <black/new/hierarchy.hpp>

  #define declare_hierarchy_element(Base, Storage, Element) \
    class Element : public Storage { \
    public: \
      static constexpr bool accepts_type(Base##_type t) { \
        return t == Base##_type::Element; \
      } \
      \
      Element(alphabet *sigma, Storage##_t *element)  \
        : Storage{sigma, element} { } \
      \
      template<typename ...Args> \
      Element(Args ...args) : Storage{Storage::type::Element, args...} { } \
    };

  #include <black/new/hierarchy.hpp>

}
