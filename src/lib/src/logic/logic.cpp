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

#include <black/logic/logic.hpp>

#include <tsl/hopscotch_map.h>

#include <boost/unordered/concurrent_flat_map.hpp>

#include <deque>
#include <mutex>
#include <variant>
#include <vector>

//
// This file contains the implementation of some components declared in
// `logic.hpp` and subfiles. In particular, here we declare some components of
// the `alphabet` class. BLACK's logic API is for the most part a header library
// being 99% templates, but this part is implemented in a source file mainly in
// order to keep the hash tables (`tsl::hopscotch_map` and
// `boost::concurrent_flat_map`) as private dependencies. To understand what
// follows, be sure to read the explanations in `core.hpp` and `generation.hpp`.
//

namespace black_internal::logic {

  } namespace std {
    template<typename T>
    struct hash<std::vector<T>>
    {
      size_t operator()(std::vector<T> const&v) const {
        hash<T> h;
        size_t result = 0;
        for(size_t i = 0; i < v.size(); ++i)
          result = ::black_internal::hash_combine(result, h(v[i]));

        return result;
      }
    };
  } namespace black_internal::logic {

  //
  // The `alphabet` class keeps an hash table from nodes to pointer to nodes.
  // When we insert a node, if it already exists, we get the existing copy of it
  // from the hash table. If it does not, we insert it in the hash table. This
  // mechanism is implemented by the *allocators* defined below, one per storage
  // kind, which will be indirectly inherited by the pimpl class
  // `alphabet_impl`.
  //
  // There are two implementations of this mechanism, chosen by the
  // `alphabet_mode` passed to the `alphabet` constructor:
  //
  // - `sequential_allocator` is the historical one. It is backed by a plain
  //   `tsl::hopscotch_map` and does no synchronization whatsoever, so the
  //   alphabet must not be touched by more than one thread at a time. This is
  //   the default, and it is left untouched on purpose: single-threaded code
  //   runs exactly the same code it ran before this distinction existed.
  //
  // - `concurrent_allocator` is backed by a `boost::concurrent_flat_map`, which
  //   does its own fine-grained internal locking, and lets several threads
  //   intern nodes into the same alphabet at once.
  //
  // The reason for keeping both, instead of just making everything concurrent,
  // is that a concurrent hash table has to pay for its synchronization even
  // when a single thread is using it, and how much it pays depends on the
  // platform, the allocator and the workload. Keeping the sequential table as
  // the default means that whatever that cost turns out to be, it is paid only
  // by the code that asks for concurrency.
  //

  //
  // The sequential allocator, i.e. verbatim what BLACK has always done.
  //
  template<storage_type Storage>
  struct sequential_allocator {
    std::deque<storage_node<Storage>> _store;
    tsl::hopscotch_map<storage_node<Storage>, storage_node<Storage> *> _map;
   
    storage_node<Storage> *allocate(storage_node<Storage> const& node) {
      auto it = _map.find(node);
      if(it != _map.end())
        return it->second;
     
      storage_node<Storage> *obj = &_store.emplace_back(node);
      _map.insert({node, obj});

      return obj;
    }
  };

  //
  // The concurrent allocator. `boost::concurrent_flat_map` spreads its own
  // locking over many small mutexes, so the common case (the node has been
  // interned already) never contends on a single alphabet-wide lock.
  //
  // `_store`, on the other hand, is a plain `std::deque`, which is *not* safe
  // for concurrent `emplace_back()`. Hence the only critical section we own is
  // the rarer path where the node is genuinely new: `_store_mutex` protects
  // that insertion alone, not the lookup. Note that pointers into a
  // `std::deque` stay valid across insertions at the end, so nodes handed out
  // earlier are never invalidated by a later concurrent insertion.
  //
  // Two threads racing to intern the *same* new node are sorted out by
  // `try_emplace_or_cvisit()`: only one of the two insertions takes place, and
  // the loser is handed the winner's pointer, so uniqueness of nodes still
  // holds. This matters because the whole hash-consing scheme, and therefore
  // `operator==` on hierarchy types, depends on it. The loser's `_store` slot
  // is simply never referenced again: a bounded and harmless amount of wasted
  // memory rather than a correctness problem, since nothing else in the
  // alphabet reaches `_store` slots by index.
  //
  template<storage_type Storage>
  struct concurrent_allocator {
    std::deque<storage_node<Storage>> _store;
    std::mutex _store_mutex;
    boost::concurrent_flat_map<
      storage_node<Storage>, storage_node<Storage> *,
      std::hash<storage_node<Storage>>
    > _map;

    storage_node<Storage> *allocate(storage_node<Storage> const& node) {
      // Fast path: the node is already there. Note the use of `cvisit()`
      // rather than `visit()`: we only read the entry, so several threads
      // looking up the same one can proceed together.
      storage_node<Storage> *found = nullptr;
      _map.cvisit(node, [&](auto const& entry) { found = entry.second; });
      if(found)
        return found;

      // Slow path: the node is new, so we have to make room for it into
      // `_store`, which is ours to protect.
      storage_node<Storage> *obj = nullptr;
      {
        std::lock_guard<std::mutex> lock(_store_mutex);
        obj = &_store.emplace_back(node);
      }

      // Then we publish it, unless somebody else got there first, in which
      // case we adopt their node and forget ours.
      storage_node<Storage> *winner = obj;
      _map.try_emplace_or_cvisit(
        node, obj, [&](auto const& entry) { winner = entry.second; }
      );

      return winner;
    }
  };

  //
  // We specialize the case of a single boolean field (i.e. the `boolean`
  // storage kind). Other optimized specializations could be possible in the
  // future. Here there is no hash table at all, only two fixed nodes that are
  // never mutated, so the same implementation serves both modes and is
  // trivially thread-safe.
  //
  template<storage_type Storage>
  concept single_boolean_storage = std::is_same_v<
    typename storage_data_t<Storage>::tuple_type, std::tuple<bool>
  >;

  template<storage_type Storage>
  struct boolean_allocator
  {
    storage_node<Storage> _true{element_of_storage_v<Storage>, true};
    storage_node<Storage> _false{element_of_storage_v<Storage>, false};

    storage_node<Storage> *allocate(storage_node<Storage> node) {
      if(std::get<0>(node.data.values))
        return &_true;
      return &_false;
    }
  };

  template<storage_type Storage>
    requires single_boolean_storage<Storage>
  struct sequential_allocator<Storage> : boolean_allocator<Storage> { };

  template<storage_type Storage>
    requires single_boolean_storage<Storage>
  struct concurrent_allocator<Storage> : boolean_allocator<Storage> { };

  //
  // An `allocator_set` holds one allocator per storage kind and merges all
  // their `allocate()` members into a single overload set. It is instantiated
  // twice, once per allocator template, which is how the two implementations
  // are kept side by side without duplicating this boilerplate.
  //
  template<template<storage_type> class Allocator>
  struct allocator_set : std::monostate
  #define declare_storage_kind(Base, Storage) \
    , Allocator<storage_type::Storage>
  #include <black/internal/logic/hierarchy.hpp>
  {
    #define declare_storage_kind(Base, Storage) \
      using Allocator<storage_type::Storage>::allocate;
    #include <black/internal/logic/hierarchy.hpp>
  };

  using sequential_allocators = allocator_set<sequential_allocator>;
  using concurrent_allocators = allocator_set<concurrent_allocator>;

  //
  // The pimpl class holds exactly one of the two sets of allocators, chosen
  // once and for all when the alphabet is created. Note that `_allocators` is
  // filled in by `emplace()` instead of by the member initializer list because
  // `concurrent_allocators` contains mutexes, hence it is neither copyable nor
  // movable.
  //
  struct alphabet_base::alphabet_impl {
    std::variant<sequential_allocators, concurrent_allocators> _allocators;

    explicit alphabet_impl(alphabet_mode mode) {
      if(mode == alphabet_mode::concurrent)
        _allocators.emplace<concurrent_allocators>();
    }

    // note that `node` is taken by reference: the allocators do the copy
    // themselves, and only when the node turns out to be a new one.
    template<storage_type Storage>
    storage_node<Storage> *allocate(storage_node<Storage> const& node) {
      if(auto *seq = std::get_if<sequential_allocators>(&_allocators); seq)
        return seq->allocate(node);

      return std::get<concurrent_allocators>(_allocators).allocate(node);
    }
  };

  //
  // Out-of-line definitions of constructors and assignments of `alphabet_base`,
  // declared in `generation.hpp`
  //
  // Note that the pimpl is created eagerly here instead of lazily on the first
  // call to `impl()`. A concurrent alphabet is meant to be shared among
  // threads, and a lazy first-touch would be a race as soon as two of them
  // interned a node at the same time. Creating it up front costs nothing
  // measurable, since the stores and the maps start out empty anyway.
  //
  alphabet_base::alphabet_base(alphabet_mode mode)
    : _mode{mode}, _impl{std::make_unique<alphabet_impl>(mode)} { }

  alphabet_base::alphabet_base(alphabet_base &&) = default;
  alphabet_base &alphabet_base::operator=(alphabet_base &&) = default;
  alphabet_base::~alphabet_base() = default;

  alphabet_mode alphabet_base::mode() const {
    return _mode;
  }

  alphabet_base::alphabet_impl *alphabet_base::impl() {
    // this can only happen on a moved-from alphabet, which by definition is
    // not shared with anybody yet, so there is no race here.
    if(!_impl)
      _impl = std::make_unique<alphabet_impl>(_mode);

    return _impl.get();
  }

  //
  // out-of-line definitions of `alphabet_base` member functions, which will be
  // inherited by `alphabet` and used by the constructors of storage and element
  // classes.
  //
  #define declare_storage_kind(Base, Storage) \
    storage_node<storage_type::Storage> * \
    alphabet_base::unique( \
      storage_node<storage_type::Storage> node \
    ) { \
      return impl()->allocate(node); \
    }

  #include <black/internal/logic/hierarchy.hpp>

}
