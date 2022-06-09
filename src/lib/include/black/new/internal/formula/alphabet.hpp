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

#ifndef BLACK_LOGIC_ALPHABET_HPP
#define BLACK_LOGIC_ALPHABET_HPP

#include <span>

namespace black::internal::new_api {

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

  //
  // class alphabet
  //
  class alphabet
  {
  public:
    alphabet();
    ~alphabet();

    alphabet(alphabet const&) = delete;
    alphabet(alphabet &&);

    alphabet &operator=(alphabet const&) = delete;
    alphabet &operator=(alphabet &&);

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
}

#endif // BLACK_LOGIC_ALPHABET_HPP
