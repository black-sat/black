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

  //
  // class alphabet, with all the allocator classes
  //
  #define declare_storage_kind(Base, Storage) \
    using Storage##_key = std::tuple<hierarchy_type,
  #define declare_field(Base, Storage, Type, Field) Type,
  #define declare_child(Base, Storage, Child) Base##_base *,
  #define end_storage_kind(Base, Storage) void*>;

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_storage { \
      std::deque<Storage##_t> Storage##_store; \
      tsl::hopscotch_map<Storage##_key, Storage##_t *> Storage##_map; \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  #define declare_storage_kind(Base, Storage) \
    struct Storage##_allocator : Storage##_storage { \
      template<typename ...Args> \
      Storage##_t *allocate_##Storage(hierarchy_type t, Args ...args) { \
        black_assert(Storage::accepts_type::doesit(t)); \
        auto it = Storage##_map.find(Storage##_key{t, args...,nullptr}); \
        if(it != Storage##_map.end()) \
          return it->second; \
        \
        Storage##_t *obj = \
          &Storage##_store.emplace_back(t, Storage##_data_t{args...}); \
        Storage##_map.insert({Storage##_key{t, args...,nullptr}, obj}); \
        \
        return obj; \
      } \
    };

  #include <black/new/internal/formula/hierarchy.hpp>

  template<typename T>
  struct dummy_t {};

  struct alphabet_impl : 
  #define declare_storage_kind(Base, Storage) Storage##_allocator,
  #include <black/new/internal/formula/hierarchy.hpp>
    dummy_t<alphabet_impl> { };

  class alphabet
  {
  public:
    alphabet() : _impl{std::make_unique<alphabet_impl>()} { }
    ~alphabet() = default;

    alphabet(alphabet const&) = delete;
    alphabet(alphabet &&) = default;

    alphabet &operator=(alphabet const&) = delete;
    alphabet &operator=(alphabet &&) = default;

    #define declare_leaf_storage_kind(Base, Storage) \
      template<typename ...Args> \
      class Storage Storage(Args ...args) { \
        return \
          ::black::internal::new_api::Storage{ \
            this, _impl->allocate_##Storage(hierarchy_type::Storage, args...) \
          }; \
      }

    #define declare_leaf_hierarchy_element(Base, Storage, Element) \
      template<typename ...Args> \
      class Element Element(Args ...args) { \
        return \
          ::black::internal::new_api::Element{ \
            this, _impl->allocate_##Storage(hierarchy_type::Element, args...) \
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
    std::unique_ptr<alphabet_impl> _impl;
  };  
}

#endif // BLACK_LOGIC_ALPHABET_HPP
