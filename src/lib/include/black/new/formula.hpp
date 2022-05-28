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

#ifndef BLACK_LOGIC_FORMULA_HPP
#define BLACK_LOGIC_FORMULA_HPP

#include <black/support/assert.hpp>
#include <black/support/hash.hpp>

#include <cstdint>

namespace black::internal::new_api 
{
  class alphabet;

  //
  // enum type of all the elements of the hierarchies
  //
  #define declare_hierarchy(Base) \
    enum class Base##_type : uint8_t { \

  #define declare_hierarchy_element(Base, Storage, Element) Element,

  #define end_declare_hierarchy(Base) \
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
      Base &operator=(Base const&) = default; \
      Base &operator=(Base &&) = default; \
      \
      type Base##_type() const { return _element->type; } \
      \
      alphabet *sigma() const { return _sigma; }  \
      \
    private: \
      alphabet *_sigma; \
      Base##_base *_element; \
    };

  #include <black/new/hierarchy.hpp>

  //
  // class allocator
  //
  #define declare_storage_kind(Base, Storage) \
    using Storage##_key = std::tuple<
  #define declare_field(Base, Storage, Type, Field) Type,
  #define declare_child(Base, Storage, Child) Base##_base *,
  #define end_storage_kind(Base, Storage) void>;

  #include <black/new/hierarchy.hpp>
  
  //
  // constexpr functions to categorize hierarchy type values into storage kinds
  //
  #define declare_storage_kind(Base, Storage) \
  constexpr bool is_##Storage##_type(Base##_type type) { \
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
      static constexpr auto accepts_type = is_##Storage##_type; \
      \
      Storage##_t(Base##_type t, Storage##_data_t _data) \
        : Base##_base{t}, data{_data} { \
          black_assert(is_##Storage##_type(t)); \
        } \
      \
      Storage##_data_t data; \
    };

  #include <black/new/hierarchy.hpp>

  //
  // cast function
  //
  #define declare_hierarchy(Base) \
    template<typename T, typename F = std::remove_pointer_t<T>> \
    F *Base##_cast(Base##_base *f) \
    { \
      black_assert(f != nullptr); \
      if(F::accepts_type(f->type)) \
        return static_cast<F *>(f); \
      return nullptr; \
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
      Type Field() const { return static_cast<H&>(*this)._formula->Field; } \

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    enum class Storage##_type : uint8_t {
  
  #define declare_hierarchy_element(Base, Storage, Element) \
      Element = to_underlying(Base##_type::Element),
  
  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    class Storage : public Storage##_fields<Storage> { \
      \
    public: \
      using type = Storage##_type; \
      \
      template<typename ...Args> \
      Storage(class alphabet *sigma, Storage##_t *element) \
        : _sigma{sigma}, _element{element} { } \
      \
      alphabet *sigma() const { return _sigma; } \
      Base##_id unique_id() const { \
        return static_cast<Base##_id>(reinterpret_cast<uintptr_t>(_element)); \
      } \
    \
    private: \
      class alphabet *_sigma; \
      Storage##_t *_element; \
    };

  #include <black/new/hierarchy.hpp>

  //
  // Helper function to call sigma() on the first argument that supports
  // the call
  //
  template<typename T, typename = void>
  struct has_sigma : std::false_type {  };

  template<typename T>
  struct has_sigma<T, std::void_t<decltype(std::declval<T>().sigma())>>
    : std::true_type { };

  template<typename T>
  alphabet *get_sigma(T v) {
    return v.sigma();
  }

  template<typename T, typename ...Args>
  alphabet *get_sigma(T v, Args ...args) {
    if constexpr(has_sigma<T>::value)
      return v.sigma();
    else
      return get_sigma(args...);
  }

  //
  // allocation functions
  //
  #define declare_leaf_storage_kind(Base, Storage)
  #define declare_storage_kind(Base, Storage) \
    template<typename ...Args> \
    Storage allocate_##Storage(Storage::type, Args ...args) { \
      \
      alphabet *sigma = get_sigma(args...); \
      \
      return Storage{sigma, nullptr}; \
    }

  #include <black/new/hierarchy.hpp>

}

#endif // BLACK_LOGIC_FORMULA_HPP
