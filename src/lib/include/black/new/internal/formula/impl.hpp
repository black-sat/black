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

#include <string_view>

namespace black::internal::new_api {

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

#endif // BLACK_INTERNAL_FORMULA_IMPL_HPP
