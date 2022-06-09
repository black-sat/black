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


  #define declare_field(Base, Storage, Type, Field) \
    template<typename H> \
    Type storage_fields_base<storage_type::Storage, H>::Field() const { \
      constexpr size_t I = index_of_field_v<storage_type::Storage, #Field>; \
      return std::get<I>(static_cast<H const&>(*this).node()->data.values); \
    }

  #define declare_child(Base, Storage, Hierarchy, Child) \
    template<typename H, fragment Syntax> \
    Hierarchy<Syntax> \
    storage_children_base<storage_type::Storage, Syntax, H>::Child() const { \
      constexpr size_t I = index_of_field_v<storage_type::Storage, #Child>; \
      return Hierarchy<Syntax>{ \
        static_cast<H const&>(*this).sigma(),  \
        std::get<I>(static_cast<H const&>(*this).node()->data.values) \
      }; \
    }

  #define declare_children(Base, Storage, Hierarchy, Children) \
    template<typename H, fragment Syntax> \
    std::vector<Hierarchy<Syntax>> \
    storage_children_base<storage_type::Storage, Syntax, H>::Children() const {\
      constexpr size_t I = index_of_field_v<storage_type::Storage, #Children>;
      std::vector<Hierarchy<Syntax>> result; \
      std::vector<hierarchy_node<hierarchy_type::Hierarchy> const*> children = \
        std::get<I>(static_cast<H const&>(*this).node()->data.values); \
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

#endif // BLACK_INTERNAL_FORMULA_IMPL_HPP
