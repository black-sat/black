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

#ifndef BLACK_INTERNAL_FORMULA_IMPL_HPP
#define BLACK_INTERNAL_FORMULA_IMPL_HPP

namespace black::internal::new_api {
  
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

  #include <black/new/internal/formula/hierarchy.hpp>

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

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_field(Base, Storage, Type, Field) \
    template<typename H> \
    Type Storage##_fields<H>::Field() const { \
      return static_cast<H const&>(*this)._element->data.Field; \
    }

  #define declare_child(Base, Storage, Child) \
    template<typename H> \
    Base Storage##_fields<H>::Child() const { \
      return Base{ \
        static_cast<H const&>(*this)._sigma,  \
        static_cast<H const&>(*this)._element->data.Child \
      }; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>


  #define declare_storage_kind(Base, Storage) \
    Base::Base(Storage const&s) : _sigma{s._sigma}, _element{s._element} { }

  #define declare_hierarchy_element(Base, Storage, Element) \
    Base::Base(Element const&e) : _sigma{e._sigma}, _element{e._element} { }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
  template<typename ...Args> \
    Storage::Storage(Args ...args) \
      : _sigma{get_sigma(args...)}, \
        _element{ \
          get_sigma(args...)->_impl->allocate_##Storage( \
            Base##_handle_args(args)... \
          ) \
        } { }
      
  #define declare_hierarchy_element(Base, Storage, Element) \
      Storage::Storage(Element const&e) : Storage{e._sigma, e._element} { }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy_element(Base, Storage, Element) \
  template<typename ...Args> \
    Element::Element(Args ...args) \
      : _sigma{get_sigma(args...)}, \
        _element{ \
          get_sigma(args...)->_impl->allocate_##Storage( \
            Base##_type::Element, \
            Base##_handle_args(args)... \
          ) \
        } { }

  #include <black/new/internal/formula/hierarchy.hpp>
}

#endif // BLACK_INTERNAL_FORMULA_IMPL_HPP