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
          h = hash_combine(h, std::hash<Hierarchy##_base *>{}(k.Child));

  #define declare_children(Base, Storage, Hierarchy, Children) \
          for(auto child : k.Children) \
            h = hash_combine(h, std::hash<Hierarchy##_base *>{}(child));

  #define end_storage_kind(Base, Storage) \
          return h; \
        } \
      }; \
    } namespace black::internal::new_api {

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_storage { \
      std::deque<Storage##_t> Storage##_store; \
      tsl::hopscotch_map<Storage##_key, Storage##_t *> Storage##_map; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    Storage##_data_t Storage##_key_to_data( \
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

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_allocator : Storage##_storage { \
      Storage##_t *allocate_##Storage(Storage##_key key) { \
        auto it = Storage##_map.find(key); \
        if(it != Storage##_map.end()) \
          return it->second; \
        \
        Storage##_t *obj = \
          &Storage##_store.emplace_back(key.type, Storage##_key_to_data(key)); \
        Storage##_map.insert({key, obj}); \
        \
        return obj; \
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
