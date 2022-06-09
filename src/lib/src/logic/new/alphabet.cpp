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

#include <black/new/formula.hpp>

#include <vector>

#include <iostream>

namespace black::internal::new_api {

  } namespace std {
    template<typename T>
    struct hash<std::vector<T>>
    {
      size_t operator()(std::vector<T> const&v) const {
        hash<T> h;
        size_t result = 0;
        for(size_t i = 0; i < v.size(); ++i)
          result = ::black::internal::hash_combine(result, h(v[i]));

        return result;
      }
    };
  } namespace black::internal::new_api {

  #define declare_storage_kind(Base, Storage) \
    inline auto node_to_tuple( \
      [[maybe_unused]] storage_node<storage_type::Storage> const& node \
    ) { \
      return std::tuple{

  #define declare_field(Base, Storage, Type, Field) \
    node.data.Field,

  #define declare_child(Base, Storage, Hierarchy, Child) \
    node.data.Child,

  #define declare_children(Base, Storage, Hierarchy, Children) \
    node.data.Children,

  #define end_storage_kind(Base, Storage) \
      }; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  template<storage_type Storage>
  using node_to_tuple_type = 
    decltype(node_to_tuple(std::declval<storage_node<Storage>>()));

  template<storage_type Storage, typename = node_to_tuple_type<Storage>>
  struct storage_allocator_ {
    std::deque<storage_node<Storage>> _store;
    tsl::hopscotch_map<storage_node<Storage>, storage_node<Storage> *> _map;
   
    storage_node<Storage> *allocate(storage_node<Storage> node) {
      auto it = _map.find(node);
      if(it != _map.end())
        return it->second;
     
      storage_node<Storage> *obj = &_store.emplace_back(node);
      _map.insert({node, obj});

      return obj;
    }
  };

  template<storage_type Storage>
  struct syntax_element_of_leaf_storage_ { };

  template<storage_type Storage>
  constexpr syntax_element syntax_element_of_leaf_storage =
    syntax_element_of_leaf_storage_<Storage>::value;

  #define declare_leaf_storage_kind(Base, Storage) \
    template<> \
    struct syntax_element_of_leaf_storage_<storage_type::Storage> { \
      static constexpr auto value = syntax_element::Storage; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  template<storage_type Storage>
  struct storage_allocator_<Storage, std::tuple<bool>> 
  {  
    storage_node<Storage> 
      _true{syntax_element_of_leaf_storage<Storage>, true};
    storage_node<Storage> 
      _false{syntax_element_of_leaf_storage<Storage>, false};
    
    storage_node<Storage> *allocate(storage_node<Storage> node) {
      if(std::get<0>(node_to_tuple(node)))
        return &_true;
      return &_false;
    }
  };

  template<storage_type Storage, typename = node_to_tuple_type<Storage>>
  struct storage_allocator : storage_allocator_<Storage> { };

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_allocator : storage_allocator<storage_type::Storage> { \
      template<typename T> \
      auto allocate_##Storage(T t) { \
        return allocate(t); \
      } \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  struct alphabet::alphabet_impl : 
  #define declare_storage_kind(Base, Storage) Storage##_allocator,
  #include <black/new/internal/formula/hierarchy.hpp>
    dummy_t { };

  alphabet::alphabet() : _impl{std::make_unique<alphabet_impl>()} { }
  alphabet::alphabet(alphabet &&) = default;
  alphabet &alphabet::operator=(alphabet &&) = default;
  alphabet::~alphabet() = default;

  #define declare_storage_kind(Base, Storage) \
    storage_node<storage_type::Storage> * \
    alphabet::allocate_##Storage(storage_node<storage_type::Storage> node)\
    { \
      return _impl->allocate_##Storage(std::move(node)); \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

}
