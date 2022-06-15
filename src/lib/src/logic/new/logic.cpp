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

#include <black/new/logic/logic.hpp>

#include <tsl/hopscotch_map.h>

#include <variant>
#include <vector>

//
// This file contains the implementation of some components declared in
// `logic.hpp` and subfiles. In particular, here we declare some components of
// the `alphabet` class. BLACK's logic API is for the most part a header library
// being 99% templates, but this part is implemented in a source file mainly in
// order to keep `tsl::hopscotch_map` as a private dependency. To understand
// what follows, be sure to read the explanations in `core.hpp` and
// `generation.hpp`.
//

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

  //
  // The `alphabet` class keeps an hash table from nodes to pointer to nodes.
  // When we insert a node, if it already exists, we get the existing copy of it
  // from the hash table. If it does not, we insert it in the hash table. This
  // mechanism is implemented in the following class, which will be indirectly
  // inherited by the pimpl class `alphabet_impl`.
  //
  template<storage_type Storage>
  struct storage_allocator {
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
  // We specialize the case of a single boolean field (i.e. the `boolean`
  // storage kind). Other optimized specializations could be possible in the
  // future.
  //
  template<storage_type Storage>
    requires (std::is_same_v<
      typename storage_data_t<Storage>::tuple_type, std::tuple<bool>
    >)
  struct storage_allocator<Storage> 
  {  
    storage_node<Storage> _true{element_of_storage_v<Storage>, true};
    storage_node<Storage> _false{element_of_storage_v<Storage>, false};
    
    storage_node<Storage> *allocate(storage_node<Storage> node) {
      if(std::get<0>(node.data.values))
        return &_true;
      return &_false;
    }
  };
  
  //
  // Here we prepare the concrete (non-template) classes exposing the member
  // functions unique_Storage (e.g. unique_boolean, unique_unary, ...).
  //
  #define declare_storage_kind(Base, Storage) \
    struct Storage##_allocator : storage_allocator<storage_type::Storage> { \
      template<typename T> \
      auto unique_##Storage(T t) { \
        return allocate(t); \
      } \
    };

  #include <black/new/internal/logic/hierarchy.hpp>

  struct alphabet_base::alphabet_impl : 
  #define declare_storage_kind(Base, Storage) Storage##_allocator,
  #include <black/new/internal/logic/hierarchy.hpp>
    std::monostate { };

  //
  // Out-of-line definitions of constructors and assignments of `alphabet_base`,
  // declared in `generation.hpp`
  //
  alphabet_base::alphabet_base() : _impl{std::make_unique<alphabet_impl>()} { }
  alphabet_base::alphabet_base(alphabet_base &&) = default;
  alphabet_base &alphabet_base::operator=(alphabet_base &&) = default;
  alphabet_base::~alphabet_base() = default;

  //
  // out-of-line definitions of `alphabet_base` member functions, which will be
  // inherited by `alphabet` and used by the constructors of storage and element
  // classes.
  //
  #define declare_storage_kind(Base, Storage) \
    storage_node<storage_type::Storage> * \
    alphabet_base::unique_##Storage(storage_node<storage_type::Storage> node)\
    { \
      return _impl->unique_##Storage(std::move(node)); \
    }

  #include <black/new/internal/logic/hierarchy.hpp>

}

//
// Here we implement the remove_booleans() function declared in `utility.hpp`.
// The implementation is split into several overloads that account for the
// different cases, which are then put together by a call to the `match()`
// function in the main function.
//
namespace black::new_api::logic {
  
  static
  formula<FO> remove_booleans(negation<FO> n, auto op) 
  {
    // !true -> false, !false -> true
    if(auto b = op.template to<boolean>(); b)
      return n.sigma()->boolean(!b->value()); 
    
    // !!p -> p
    if(auto nop = op.template to<negation<FO>>(); nop) 
      return remove_booleans(nop->argument());

    return !remove_booleans(op);
  }

  static
  formula<FO> remove_booleans(conjunction<FO> c, auto l, auto r) {
    alphabet &sigma = *c.sigma();
    std::optional<boolean> bl = l.template to<boolean>(); 
    std::optional<boolean> br = r.template to<boolean>();

    if(!bl && !br)
      return c;

    if(bl && !br) {
      return bl->value() ? remove_booleans(r) : sigma.bottom();
    }
    
    if(!bl && br)
      return br->value() ? remove_booleans(l) : sigma.bottom();

    return sigma.boolean(bl->value() && br->value());
  }

  static
  formula<FO> remove_booleans(disjunction<FO> d, auto l, auto r) {
    alphabet &sigma = *d.sigma();
    std::optional<boolean> bl = l.template to<boolean>();
    std::optional<boolean> br = r.template to<boolean>();

    if(!bl && !br)
      return d;

    if(bl && !br)
      return bl->value() ? sigma.top() : remove_booleans(r);
    
    if(!bl && br)
      return br->value() ? sigma.top() : remove_booleans(l);
      
    return sigma.boolean(bl->value() || br->value());
  }

  static
  formula<FO> remove_booleans(implication<FO> t, auto l, auto r) {
    alphabet &sigma = *t.sigma();
    std::optional<boolean> bl = l.template to<boolean>();
    std::optional<boolean> br = r.template to<boolean>();

    if(!bl && !br)
      return t;

    if(bl && !br)
      return bl->value() ? r : sigma.top();
    
    if(!bl && br)
      return br->value() ? formula{sigma.top()} : remove_booleans(!l);

    return sigma.boolean(!bl->value() || br->value());
  }

  static
  formula<FO> remove_booleans(iff<FO> f, auto l, auto r) {
    alphabet &sigma = *f.sigma();
    std::optional<boolean> bl = l.template to<boolean>();
    std::optional<boolean> br = r.template to<boolean>();

    if(!bl && !br)
      return f;

    if(bl && !br)
      return bl->value() ? remove_booleans(r) : remove_booleans(!r);
    
    if(!bl && br)
      return br->value() ? remove_booleans(l) : remove_booleans(!l);

    return sigma.boolean(bl->value() == br->value());
  }

  formula<FO> remove_booleans(formula<FO> f) {
    return f.match( // LCOV_EXCL_LINE
      [](boolean b)     -> formula<FO> { return b; },
      [](proposition p) -> formula<FO> { return p; },
      [](atom<FO> a)    -> formula<FO> { return a; },
      [](exists<FO> q)  -> formula<FO> { 
        return exists<FO>(q.var(), remove_booleans(q.matrix())); 
      },
      [](forall<FO> q)  -> formula<FO> { 
        return forall<FO>(q.var(), remove_booleans(q.matrix())); 
      },
      [](comparison<FO> c) -> formula<FO> {
        return c;
      },
      [](auto ...args) -> formula<FO> {
        return remove_booleans(args...);
      }
    );
}

}
