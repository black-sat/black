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

  #define declare_child(Base, Storage, Hierarchy, Child) \
    Hierarchy##_base *Child;
  
  #define declare_children(Base, Storage, Hierarchy, Children) \
    std::vector<Hierarchy##_base *> Children;

  #define end_storage_kind(Base, Storage)  \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_t : Base##_base { \
      \
      Storage##_t(syntax_element t, Storage##_data_t _data) \
        : Base##_base{t}, data{_data} { } \
      \
      Storage##_data_t data; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_field(Base, Storage, Type, Field) \
    template<typename H> \
    Type Storage##_fields<H>::Field() const { \
      return static_cast<H const&>(*this)._element->data.Field; \
    }

  #define declare_child(Base, Storage, Hierarchy, Child) \
    template<typename Syntax, typename H> \
    Hierarchy<Syntax> Storage##_children<Syntax, H>::Child() const { \
      return Hierarchy<Syntax>{ \
        static_cast<H const&>(*this)._sigma,  \
        static_cast<H const&>(*this)._element->data.Child \
      }; \
    }

  #define declare_children(Base, Storage, Hierarchy, Children) \
    template<typename Syntax, typename H> \
    std::vector<Hierarchy<Syntax>> \
    Storage##_children<Syntax, H>::Children() const { \
      std::vector<Hierarchy<Syntax>> result; \
      std::vector<Hierarchy##_base *> children = \
        static_cast<H const&>(*this)._element->data.Children; \
      alphabet *sigma = static_cast<H const&>(*this)._sigma; \
      \
      for(auto child : children) \
        result.push_back(Hierarchy<Syntax>{sigma, child}); \
      \
      return result; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>


  #define declare_hierarchy(Base) \
    template<typename Syntax> \
    template< \
      typename Syntax2, \
      REQUIRES_OUT_OF_LINE(is_syntax_allowed<Syntax2, Syntax>) \
    > \
    Base<Syntax>::Base(Base<Syntax2> const& b) \
      : _sigma{b._sigma}, _element{b._element} { } \
    \
    template<typename Syntax> \
    template< \
      typename Syntax2, \
      REQUIRES_OUT_OF_LINE(is_syntax_allowed<Syntax2, Syntax>) \
    > \
    Base<Syntax> &Base<Syntax>::operator=(Base<Syntax2> const& b) { \
      _sigma = b._sigma; \
      _element = b._element; \
      \
      return *this; \
    }

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    template< \
      typename Syntax2, \
      REQUIRES_OUT_OF_LINE(is_syntax_allowed<Syntax2, Syntax>) \
    > \
    Base<Syntax>::Base(Storage<Syntax2> const&s) \
      : _sigma{s._sigma}, _element{s._element} { }

  #define declare_leaf_storage_kind(Base, Storage) \
    template<typename Syntax> \
    template< \
      REQUIRES_OUT_OF_LINE(is_type_allowed<syntax_element::Storage, Syntax>) \
    > \
    Base<Syntax>::Base(Storage const&s) \
      : _sigma{s._sigma}, _element{s._element} { }

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    template< \
      typename Syntax2, \
      REQUIRES_OUT_OF_LINE(is_syntax_allowed<Syntax2, Syntax>) \
    > \
    Base<Syntax>::Base(Element<Syntax2> const&e) \
       : _sigma{e._sigma}, _element{e._element} { }

  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    template< \
      REQUIRES_OUT_OF_LINE(is_type_allowed<syntax_element::Element, Syntax>) \
    > \
    Base<Syntax>::Base(Element const&e) \
      : _sigma{e._sigma}, _element{e._element} { }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
   constexpr syntax_element Storage##_syntax_element() { \
     return 
  
  #define has_no_hierarchy_elements(Base, Storage) \
    true ? syntax_element::Storage : 

  #define end_storage_kind(Base, Storage) \
     syntax_element::no_type; \
   }

  #define declare_leaf_storage_kind(Base, Storage) \
    constexpr syntax_element Storage##_syntax_element() { \
      return syntax_element::Storage; \
    }

  #define end_leaf_storage_kind(Base, Storage)

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_leaf_storage_kind(Base, Storage)
  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    template< \
      typename ...Args, \
      REQUIRES_OUT_OF_LINE( \
        (Storage##_has_hierarchy_elements() && \
        (is_argument_allowed<Args, Syntax> && ...)) \
      ) \
    > \
    Storage<Syntax>::Storage(type t, Args ...args) \
      : _sigma{get_sigma(args...)}, \
        _element{ \
          get_sigma(args...)->allocate_##Storage( \
            Storage##_args_to_key<Syntax>({t.type(), args...}) \
          ) \
        } { } \
    \
    template<typename Syntax> \
    template< \
      typename ...Args, \
      REQUIRES_OUT_OF_LINE( \
        (!Storage##_has_hierarchy_elements() && \
        (is_argument_allowed<Args, Syntax> && ...)) \
      ) \
    > \
    Storage<Syntax>::Storage(Args ...args) \
      : _sigma{get_sigma(args...)}, \
        _element{ \
          get_sigma(args...)->allocate_##Storage( \
            Storage##_args_to_key( \
              Storage##_alloc_args<Storage::syntax>{ \
                Storage##_syntax_element(), args... \
              } \
            ) \
          ) \
        } { }
      
  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    template< \
      typename Syntax2, \
      REQUIRES_OUT_OF_LINE(is_syntax_allowed<Syntax2, Syntax>) \
    > \
    Storage<Syntax>::Storage(Element<Syntax2> const&e) \
      : Storage{e._sigma, e._element} { }
  
  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    Storage<Syntax>::Storage(Element const&e) \
      : Storage{e._sigma, e._element} { }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    template< \
        typename ...Args, \
        REQUIRES_OUT_OF_LINE((is_argument_allowed<Args, Syntax> && ...)) \
      > \
    Element<Syntax>::Element(Args ...args) \
      : _sigma{get_sigma(args...)}, \
        _element{ \
          get_sigma(args...)->allocate_##Storage( \
            Storage##_args_to_key( \
              Storage##_alloc_args<Syntax>{syntax_element::Element, args...} \
            ) \
          ) \
        } { } \
    \
    template<typename Syntax> \
    Element<Syntax>::Element(class alphabet *sigma, Storage##_t *element) \
        : _sigma{sigma}, _element{element} { \
          black_assert(_element->type == syntax_element::Element); \
        } 

  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    Element::Element(class alphabet *sigma, Storage##_t *element) \
        : _sigma{sigma}, _element{element} { \
          black_assert(_element->type == syntax_element::Element); \
        } 

  #include <black/new/internal/formula/hierarchy.hpp>

}

#endif // BLACK_INTERNAL_FORMULA_IMPL_HPP
