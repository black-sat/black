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

namespace black::internal::new_api {
  
  #define declare_storage_kind(Base, Storage) \
    struct Storage##_key { \
      syntax_element type;

  #define declare_field(Base, Storage, Type, Field) Type Field;

  #define declare_child(Base, Storage, Hierarchy, Child) \
    Hierarchy##_base *Child;

  #define declare_children(Base, Storage, Hierarchy, Children) \
    std::vector<Hierarchy##_base *> Children;

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    inline bool operator==( \
      [[maybe_unused]] Storage##_key k1, \
      [[maybe_unused]] Storage##_key k2 \
    ) { \
      return 

  #define declare_field(Base, Storage, Type, Field) k1.Field == k2.Field &&

  #define declare_child(Base, Storage, Hierarchy, Child) k1.Child == k2.Child &&

  #define declare_children(Base, Storage, Hierarchy, Children) \
    k1.Children == k2.Children &&
  
  #define end_storage_kind(Base, Storage) \
        true; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  template<typename Syntax, hierarchy_type Hierarchy>
  struct children_vector 
  {
    template<
      typename H, 
      REQUIRES(
        H::hierarchy == Hierarchy && 
        is_syntax_allowed<typename H::syntax, Syntax>
      )
    >
    children_vector(std::vector<H> const& v) {
      for(auto h : v)
        children.push_back(h._element);
    }

    std::vector<hierarchy_base_type_of<Hierarchy> *> children;
  };

  #define declare_leaf_storage_kind(Base, Storage)
  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax, bool B = true> \
    struct Storage##_alloc_args_type { \
      int x; \
    }; \
    \
    template<typename Syntax> \
    struct Storage##_alloc_args_type< \
      Syntax, \
      Storage##_has_hierarchy_elements() \
    > { \
      int x; \
      typename Storage<Syntax>::type type; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
    struct Storage##_alloc_args : Storage##_alloc_args_type<Syntax> { \
  
  #define declare_leaf_storage_kind(Base, Storage) \
    template<typename Syntax> \
    struct Storage##_alloc_args { \
      int x;

  #define declare_field(Base, Storage, Type, Field) Type Field;

  #define declare_child(Base, Storage, Hierarchy, Child) \
    Hierarchy<Syntax> Child;

  #define declare_children(Base, Storage, Hierarchy, Children) \
    children_vector<Syntax, hierarchy_type::Hierarchy> Children;

  #define end_storage_kind(Base, Storage) \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_hierarchy(Base) \
    template<typename T> \
    auto Base##_children_to_key(std::vector<T> const& v) \
    { \
      std::vector<Base##_base *> result; \
      for(auto x : v) { \
        result.push_back(x._element); \
      } \
      return result; \
    }
  
  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax, REQUIRES(Storage##_has_hierarchy_elements())> \
    Storage##_key Storage##_args_to_key( \
      Storage##_alloc_args<Syntax> const&args \
    ) { \
      return Storage##_key { \
        args.type.type(),

  #define declare_field(Base, Storage, Type, Field) args.Field,

  #define declare_child(Base, Storage, Hierarchy, Child) args.Child._element,

  #define declare_children(Base, Storage, Hierarchy, Children) \
    args.Children.children,

  #define end_storage_kind(Base, Storage) \
      }; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax, REQUIRES(!Storage##_has_hierarchy_elements())> \
    Storage##_key Storage##_args_to_key( \
      [[maybe_unused]] Storage##_alloc_args<Syntax> const&args \
    ) { \
      return Storage##_key { \
        syntax_element::no_type,

  #define declare_field(Base, Storage, Type, Field) args.Field,

  #define declare_child(Base, Storage, Hierarchy, Child) args.Child._element,

  #define declare_children(Base, Storage, Hierarchy, Children) \
    args.Children.children,

  #define end_storage_kind(Base, Storage) \
      }; \
   }

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
      class Storage Storage(Args ...args) { \
        return \
          ::black::internal::new_api::Storage{ \
            this, \
            allocate_##Storage( \
              Storage##_key{syntax_element::Storage, args...} \
            ) \
          }; \
      }

    #define declare_leaf_hierarchy_element(Base, Storage, Element) \
      template<typename ...Args> \
      class Element Element(Args ...args) { \
        return \
          ::black::internal::new_api::Element{ \
            this, \
            allocate_##Storage( \
              Storage##_key{syntax_element::Element, args...} \
            ) \
          }; \
      }

    #include <black/new/internal/formula/hierarchy.hpp>

    #define declare_storage_kind(Base, Storage) \
      template<typename Syntax> \
      friend class Storage;
    #define declare_leaf_storage_kind(Base, Storage) \
      friend class Storage;
    #define declare_hierarchy_element(Base, Storage, Element) \
      template<typename Syntax> \
      friend class Element;
    #define declare_leaf_hierarchy_element(Base, Storage, Element) \
      friend class Element;
    #include <black/new/internal/formula/hierarchy.hpp>

  private:
    #define declare_storage_kind(Base, Storage) \
      Storage##_t *allocate_##Storage(Storage##_key key);

    #include <black/new/internal/formula/hierarchy.hpp>

    struct alphabet_impl;
    std::unique_ptr<alphabet_impl> _impl;
  };  
}

#endif // BLACK_LOGIC_ALPHABET_HPP
