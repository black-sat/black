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
    struct Storage##_storage { \
      std::deque<Storage##_t> Storage##_store; \
      tsl::hopscotch_map<Storage##_key, Storage##_t *> Storage##_map; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  template<typename Data, typename ...Types, size_t ...Indexes>
  Data key_to_data(
    std::tuple<Types...> t, 
    std::index_sequence<Indexes...>) 
  {
    return Data{std::get<Indexes + 1>(t)...};
  }

  template<typename Data, typename ...Types>
  Data key_to_data(std::tuple<Types...> t) 
  {
    return key_to_data<Data>(
      t, std::make_index_sequence<sizeof...(Types) - 2>{}
    );
  }

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_allocator : Storage##_storage { \
      Storage##_t *allocate_##Storage(Storage##_key key) { \
        auto it = Storage##_map.find(key); \
        if(it != Storage##_map.end()) \
          return it->second; \
        \
        Storage##_t *obj = \
          &Storage##_store.emplace_back( \
            std::get<0>(key), key_to_data<Storage##_data_t>(key) \
          ); \
        Storage##_map.insert({key, obj}); \
        \
        return obj; \
      } \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  struct dummy_t {};
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
