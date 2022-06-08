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

namespace black::internal::new_api {

  #define declare_storage_kind(Base, Storage) \
    } namespace std { \
      template<> \
      struct hash<::black::internal::new_api::Storage##_key> { \
        size_t operator()( \
          ::black::internal::new_api::Storage##_key const&k \
        ) const { \
          using namespace ::black::internal::new_api; \
          using namespace ::black::internal; \
          using namespace ::black; \
          size_t h = std::hash<syntax_element>{}(k.type);

  #define declare_field(Base, Storage, Type, Field) \
          h = hash_combine(h, std::hash<Type>{}(k.Field));

  #define declare_child(Base, Storage, Hierarchy, Child) \
          h = hash_combine(h, \
            std::hash<hierarchy_base<hierarchy_type::Hierarchy> *>{}(k.Child) \
          );

  #define declare_children(Base, Storage, Hierarchy, Children) \
          for(auto child : k.Children) \
            h = hash_combine(h, \
              std::hash<hierarchy_base<hierarchy_type::Hierarchy> *>{}(child) \
            );

  #define end_storage_kind(Base, Storage) \
          return h; \
        } \
      }; \
    } namespace black::internal::new_api {

  #include <black/new/internal/formula/hierarchy.hpp>

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
    inline Storage##_data_t key_to_data( \
      [[maybe_unused]]Storage##_key const& k \
    ) { \
      return Storage##_data_t {

  #define declare_field(Base, Storage, Type, Field) k.Field,

  #define declare_child(Base, Storage, Hierarchy, Child) k.Child,

  #define declare_children(Base, Storage, Hierarchy, Children) k.Children,
  
  #define end_storage_kind(Base, Storage) \
      }; \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  template<storage_type Storage>
  struct storage_key_of_;

  template<storage_type Storage>
  using storage_key_of = typename storage_key_of_<Storage>::type;

  #define declare_storage_kind(Base, Storage) \
    template<> \
    struct storage_key_of_<storage_type::Storage> { \
      using type = Storage##_key; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    inline auto key_to_tuple([[maybe_unused]] Storage##_key key) { \
      return std::make_tuple(

  #define declare_field(Base, Storage, Type, Field) key.Field,

  #define declare_child(Base, Storage, Hierarchy, Child) key.Child,

  #define declare_children(Base, Storage, Hierarchy, Children) key.Children,

  #define end_storage_kind(Base, Storage) \
      0); \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

  template<storage_type Storage>
  using key_to_tuple_type = 
    decltype(key_to_tuple(std::declval<storage_key_of<Storage>>()));

  template<storage_type Storage, typename = key_to_tuple_type<Storage>>
  struct storage_allocator_ {
    using storage_t = storage_base_type_of<Storage>;
    using storage_key = storage_key_of<Storage>;
    
    std::deque<storage_t> _store;
    tsl::hopscotch_map<storage_key, storage_t *> _map;
   
    storage_t *allocate(storage_key key) {
      auto it = _map.find(key);
      if(it != _map.end())
        return it->second;
     
      storage_t *obj =
        &_store.emplace_back(key.type, key_to_data(key));
      _map.insert({key, obj});
     
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
  struct storage_allocator_<Storage, std::tuple<bool, int>> {
    using storage_t = storage_base_type_of<Storage>;
    using storage_key = storage_key_of<Storage>;

    storage_t _true{syntax_element_of_leaf_storage<Storage>, {true}};
    storage_t _false{syntax_element_of_leaf_storage<Storage>, {false}};
    
    storage_t *allocate(storage_key key) {
      if(std::get<0>(key_to_tuple(key)))
        return &_true;
      return &_false;
    }
  };

  template<storage_type Storage, typename = key_to_tuple_type<Storage>>
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
    Storage##_t *alphabet::allocate_##Storage(Storage##_key key) { \
      return _impl->allocate_##Storage(key); \
    }

  #include <black/new/internal/formula/hierarchy.hpp>

}
