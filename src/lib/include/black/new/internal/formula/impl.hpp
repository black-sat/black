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
    template<> \
    struct storage_data_t<storage_type::Storage> {

  #define declare_field(Base, Storage, Type, Field) \
    Type Field;

  #define declare_child(Base, Storage, Hierarchy, Child) \
    hierarchy_node<hierarchy_type::Hierarchy> const *Child;
  
  #define declare_children(Base, Storage, Hierarchy, Children) \
    std::vector<hierarchy_node<hierarchy_type::Hierarchy> const*> Children;

  #define end_storage_kind(Base, Storage)  \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_field(Base, Storage, Type, Field) \
    template<typename H> \
    Type Storage##_fields<H>::Field() const { \
      return static_cast<H const&>(*this).node()->data.Field; \
    }

  #define declare_child(Base, Storage, Hierarchy, Child) \
    template<typename Syntax, typename H> \
    Hierarchy<Syntax> Storage##_children<Syntax, H>::Child() const { \
      return Hierarchy<Syntax>{ \
        static_cast<H const&>(*this).sigma(),  \
        static_cast<H const&>(*this).node()->data.Child \
      }; \
    }

  #define declare_children(Base, Storage, Hierarchy, Children) \
    template<typename Syntax, typename H> \
    std::vector<Hierarchy<Syntax>> \
    Storage##_children<Syntax, H>::Children() const { \
      std::vector<Hierarchy<Syntax>> result; \
      std::vector<hierarchy_node<hierarchy_type::Hierarchy> const*> children = \
        static_cast<H const&>(*this).node()->data.Children; \
      alphabet *sigma = static_cast<H const&>(*this).sigma(); \
      \
      for(auto child : children) \
        result.push_back(Hierarchy<Syntax>{sigma, child}); \
      \
      return result; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>


  #define declare_leaf_storage_kind(Base, Storage)
  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    template< \
      typename ...Args, \
      REQUIRES_OUT_OF_LINE(is_##Storage##_constructible<Syntax, Args...>) \
    > \
    Storage<Syntax>::Storage(Args ...args) \
      : _sigma{get_sigma(args...)}, \
        _node{ \
          get_sigma(args...)->allocate_##Storage( \
            Storage##_args_to_key<Syntax>( \
              Storage##_alloc_args<Syntax>{0, args...} \
            ) \
          ) \
        } { } \
    \
    template<typename Syntax> \
      template< \
          typename H, \
          REQUIRES_OUT_OF_LINE( \
            H::storage == storage_type::Storage && \
            is_subfragment_of_v<typename H::syntax, Syntax> \
          ) \
        > \
        Storage<Syntax>::Storage(H const&e) \
          : _sigma{e._sigma}, _node{e._node} { }

  #define declare_hierarchy_element(Base, Storage, Element) \
    template<typename Syntax> \
    template< \
        typename ...Args, \
        REQUIRES_OUT_OF_LINE(is_##Element##_constructible<Syntax, Args...>) \
      >  \
    Element<Syntax>::Element(Args ...args) \
      : _sigma{get_sigma(args...)}, \
        _node{ \
          get_sigma(args...)->allocate_##Storage( \
            Storage##_args_to_key( \
              Storage##_alloc_args<Syntax>{0, \
                Storage<Syntax>::type::Element, \
                args... \
              } \
            ) \
          ) \
        } { } \
    \
    template<typename Syntax> \
    template<typename S, REQUIRES_OUT_OF_LINE(is_subfragment_of_v<S, Syntax>)> \
    Element<Syntax>::Element(Element<S> e) \
      : _sigma{e._sigma}, _node{e._node} { } \
    \
    template<typename Syntax> \
    Element<Syntax>::Element(class alphabet *sigma, storage_node<storage_type::Storage> const*node) \
        : _sigma{sigma}, _node{node} { \
          black_assert(_node->type == syntax_element::Element); \
        } 

  #define declare_leaf_hierarchy_element(Base, Storage, Element) \
    inline Element::Element(class alphabet *sigma, storage_node<storage_type::Storage> const*node) \
        : _sigma{sigma}, _node{node} { \
          black_assert(_node->type == syntax_element::Element); \
        } 

  #include <black/new/internal/formula/hierarchy.hpp>

}

#endif // BLACK_INTERNAL_FORMULA_IMPL_HPP
