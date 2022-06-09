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
    inline bool operator==( \
      [[maybe_unused]] storage_data_t<storage_type::Storage> const& k1, \
      [[maybe_unused]] storage_data_t<storage_type::Storage> const& k2 \
    ) { \
      return 

  #define declare_field(Base, Storage, Type, Field) \
    are_equal(k1.Field, k2.Field) &&

  #define declare_child(Base, Storage, Hierarchy, Child) \
    k1.Child == k2.Child &&

  #define declare_children(Base, Storage, Hierarchy, Children) \
    k1.Children == k2.Children &&
  
  #define end_storage_kind(Base, Storage) \
        true; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    } namespace std { \
      template<> \
      struct hash<::black::internal::new_api::storage_data_t< \
        ::black::internal::new_api::storage_type::Storage \
      >> { \
        size_t operator()( \
          [[maybe_unused]] \
          ::black::internal::new_api::storage_data_t< \
              ::black::internal::new_api::storage_type::Storage \
            > const& k \
        ) const { \
          using namespace ::black::internal; \
          using namespace ::black::internal::new_api; \
          size_t h = 0;

  #define declare_field(Base, Storage, Type, Field) \
          h = hash_combine(h, std::hash<Type>{}(k.Field));

  #define declare_child(Base, Storage, Hierarchy, Child) \
          h = hash_combine(h, \
            std::hash<hierarchy_node<hierarchy_type::Hierarchy> const*>{}( \
              k.Child \
            ) \
          );

  #define declare_children(Base, Storage, Hierarchy, Children) \
          for(auto child : k.Children) \
            h = hash_combine(h, \
              std::hash<hierarchy_node<hierarchy_type::Hierarchy> const *>{}( \
                child \
              ) \
            );

  #define end_storage_kind(Base, Storage) \
          return h; \
        } \
      }; \
    } namespace black::internal::new_api {

  #include <black/new/internal/formula/hierarchy.hpp>

  template<fragment Syntax, hierarchy_type Hierarchy>
  struct children_vector 
  {
    template<std::ranges::range R>
    using value_t = std::ranges::range_value_t<R>;

    template<std::ranges::range R>
      requires (value_t<R>::hierarchy == Hierarchy && 
                is_subfragment_of_v<typename value_t<R>::syntax, Syntax>)
    children_vector(R v) {
      for(auto h : v)
        children.push_back(h.node());
    }

    std::vector<hierarchy_node<Hierarchy> const*> children;
  };

  #define declare_storage_kind(Base, Storage) \
    template<fragment Syntax> \
    struct storage_alloc_args<Syntax, storage_type::Storage> \
      : storage_alloc_args_base<Syntax, storage_type::Storage> { \
  
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
      std::vector<hierarchy_node<hierarchy_type::Base> *> result; \
      for(auto x : v) { \
        result.push_back(x._element); \
      } \
      return result; \
    }
  
  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
      requires storage_has_hierarchy_elements_v<storage_type::Storage> \
    storage_node<storage_type::Storage> args_to_node( \
      storage_alloc_args<Syntax, storage_type::Storage> const&args \
    ) { \
      return storage_node<storage_type::Storage> { \
        syntax_element(args.type),

  #define declare_field(Base, Storage, Type, Field) args.Field,

  #define declare_child(Base, Storage, Hierarchy, Child) args.Child.node(),

  #define declare_children(Base, Storage, Hierarchy, Children) \
    args.Children.children,

  #define end_storage_kind(Base, Storage) \
      }; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
   constexpr std::optional<syntax_element> Storage##_syntax_element() { \
     return 
  
  #define has_no_hierarchy_elements(Base, Storage) \
    true ? syntax_element::Storage : 

  #define end_storage_kind(Base, Storage) \
     std::optional<syntax_element>{}; \
   }

  #define declare_leaf_storage_kind(Base, Storage) \
    constexpr std::optional<syntax_element> Storage##_syntax_element() { \
      return syntax_element::Storage; \
    }

  #define end_leaf_storage_kind(Base, Storage)

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    template<typename Syntax> \
      requires (!storage_has_hierarchy_elements_v<storage_type::Storage>) \
    storage_node<storage_type::Storage> args_to_node( \
      [[maybe_unused]] \
      storage_alloc_args<Syntax, storage_type::Storage> const&args \
    ) { \
      black_assert(Storage##_syntax_element().has_value()); \
      return storage_node<storage_type::Storage> { \
        *Storage##_syntax_element(),

  #define declare_field(Base, Storage, Type, Field) args.Field,

  #define declare_child(Base, Storage, Hierarchy, Child) args.Child.node(),

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
