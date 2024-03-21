//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#include <black/support>
#include <black/logic>

#include <immer/vector.hpp>
#include <immer/flex_vector.hpp>
#include <immer/map.hpp>
#include <immer/set.hpp>
#include <immer/algorithm.hpp>

#include <algorithm>

namespace black::logic {

  //
  // Recursive definitions
  //
  // 1. adopt() pushes directly to the stack, not to the pending set
  // 2. resolve() has a new argument to choose linear or recursive scope
  // 3. if the scope is linear the lookups are not popped from the pending set
  //    before resolving
  // 4. resolution checks for possible recursion 
  //    a. not an actual DFS (for now)
  //    b. we just check if the objects added from the pending set are actually 
  //       used during resolve()
  //    c. in this case we set recursive = true in the SCC
  // 5. in this way we can:
  //    - choose the right way to declare things in the backend
  //    - choose `scope::linear` or `scope::recursive` appropriately during 
  //      replay
  // 6. objects pushed from pending with scope::recursive form an SCC with 
  //    recursive = true only if we have scope::recursive and item 4 detected 
  //    possible recursion
  // 7. in this case adopt(vector<object>) is replayed
  // 8. otherwise, a sequence of single adopt(object) are replayed
  // 9. memory management is solved by using a weak reference in `object` only 
  //    if a back-edge is detected at step 4.
  //
  struct module::_impl_t 
  {
    struct scc_t {
      bool recursive;
      immer::set<std::shared_ptr<struct lookup const>> lookups;

      bool operator==(scc_t const&) const = default;
    };
    
    struct frame_t {
      immer::vector<module> imports;
      immer::vector<scc_t> sccs;
      immer::map<variable, struct lookup const*> scope;
      immer::vector<term> reqs;

      bool operator==(frame_t const&) const = default;
    };
    
    immer::vector<frame_t> stack;
    immer::set<std::shared_ptr<struct lookup>> pending;

    _impl_t() : stack{frame_t{}} { }

    bool operator==(_impl_t const&) const = default;

    std::optional<frame_t>
    diff(frame_t const& ours, frame_t const& theirs) const;
    
    void replay(frame_t const& f, replay_target_t *target) const;
    void replay(module const &from, replay_target_t *target) const;
  };

  module::module() : _impl{std::make_unique<_impl_t>()} { } 

  module::module(module const& other)
    : _impl{std::make_unique<_impl_t>(_impl_t{*other._impl})} { }

  module::module(module &&) = default;

  module::~module() = default;

  module &module::operator=(module const& other) {
    *_impl = *other._impl;
    return *this;
  }

  module &module::operator=(module &&) = default;

  bool module::operator==(module const& other) const {
    return *_impl == *other._impl;
  }

  void module::import(module m) {
    _impl->stack = _impl->stack.update(_impl->stack.size() - 1, [&](auto top) {
      top.imports = top.imports.push_back(m.resolved());
      return top;
    });
  }

  object module::declare(decl d, resolution r) {
    auto ptr = std::make_shared<struct lookup>(d);
    _impl->pending = _impl->pending.insert(ptr);
    
    if(r == resolution::immediate)
      resolve();

    return object(ptr);
  }
  
  object module::define(def d, resolution r) {
    auto ptr = std::make_shared<struct lookup>(d);
    _impl->pending = _impl->pending.insert(ptr);
    
    if(r == resolution::immediate)
      resolve();

    return object(ptr);
  }

  object module::define(function_def f, resolution r) {
    std::vector<term> argtypes;
    for(decl d : f.parameters)
      argtypes.push_back(d.type);

    auto type = function_type(argtypes, f.range);
    auto body = lambda(f.parameters, f.body);

    return define(def{f.name, type, body}, r);
  }

  void module::adopt(std::vector<object> const& objs, scope s) {
    _impl_t::scc_t scc = {s == scope::recursive, {}};
    for(object obj : objs) {
      auto lu = obj.lookup()->shared_from_this();
      scc.lookups = scc.lookups.insert(lu);
    }
    _impl->stack = _impl->stack.update(_impl->stack.size() - 1, [&](auto top) {
      top.sccs = top.sccs.push_back(scc);
      for(auto lu : scc.lookups)
        top.scope = top.scope.set(lu->name, lu.get());
      return top;
    });
  }

  std::optional<object> module::lookup(variable s) const 
  {
    for(auto it = _impl->stack.rbegin(); it != _impl->stack.rend(); it++)
      if(auto p = it->scope.find(s); p)
        return object((*p)->shared_from_this());
    
    for(auto it = _impl->stack.rbegin(); it != _impl->stack.rend(); it++) 
      for(auto im = it->imports.rbegin(); im != it->imports.rend(); im++)
        if(auto result = im->lookup(s); result)
          return result;   
    
    return {};
  }

  void module::require(term req) {
    black_assert(!_impl->stack.empty());
    _impl->stack = _impl->stack.update(_impl->stack.size() - 1, [&](auto top) {
      top.reqs = top.reqs.push_back(req);
      return top;
    });
  }

  void module::push() {
    _impl->stack = _impl->stack.push_back({});
  }

  void module::pop(size_t n) {
    black_assert(!_impl->stack.empty());
    _impl->stack = _impl->stack.take(_impl->stack.size() - n);
    if(_impl->stack.empty())
      _impl->stack = _impl->stack.push_back({});
  }

  auto module::_impl_t::diff(frame_t const&inner, frame_t const&outer) const 
    -> std::optional<frame_t>
  {
    frame_t result;
    
    // check vectors sizes
    if(inner.imports.size() > outer.imports.size())
      return {};
    if(inner.sccs.size() > outer.sccs.size())
      return {};
    if(inner.reqs.size() > outer.reqs.size())
      return {};

    // check that inner is a prefix
    for(size_t i = 0; i < inner.imports.size(); i++)
      if(inner.imports[i] != outer.imports[i])
        return {};
    
    for(size_t i = 0; i < inner.sccs.size(); i++)
      if(inner.sccs[i] != outer.sccs[i])
        return {};
    
    for(size_t i = 0; i < inner.reqs.size(); i++)
      if(inner.reqs[i] != outer.reqs[i])
        return {};
    
    // collect the additional one from outer
    for(size_t i = inner.imports.size(); i < outer.imports.size(); i++)
      result.imports = result.imports.push_back(outer.imports[i]);
    
    for(size_t i = inner.sccs.size(); i < outer.sccs.size(); i++)
      result.sccs = result.sccs.push_back(outer.sccs[i]);
    
    for(size_t i = inner.reqs.size(); i < outer.reqs.size(); i++)
      result.reqs = result.reqs.push_back(outer.reqs[i]);

    return result;
  }
    
  void 
  module::_impl_t::replay(frame_t const& f, replay_target_t *target) const {
    for(module m : f.imports)
      target->import(std::move(m));
    
    for(scc_t scc : f.sccs) {
      std::vector<object> objs;
      for(auto lu : scc.lookups)
        objs.push_back(object(lu));
      target->adopt(objs, scc.recursive ? scope::recursive : scope::linear);
    }
    
    for(term req : f.reqs)
      target->require(req);
  }

  void 
  module::_impl_t::replay(module const&from, replay_target_t *target) const {
    auto ours = stack;
    auto theirs = from._impl->stack;
    size_t shortest = std::min(ours.size(), theirs.size());

    // 'start' is the index of the first different frame in `ours` and `theirs`
    // after the longest common prefix
    // if `start == shortest` one of the two is a strict subset of the other
    size_t start = [&]{
      for(size_t i = 0; i < shortest; i++)
        if(ours[i] != theirs[i]) // TODO: study the performance of this
          return i;
      return shortest;
    }();

    // pop the extra levels except `theirs[start]`
    size_t different = theirs.size() - std::min(theirs.size(), start);
    if(different > 1)
      target->pop(different - 1);

    // if ours is a prefix of theirs, we pop `theirs[start]` as well,
    // and we are done
    if(start == ours.size()) {
      target->pop(1);
      return;
    }

    // if no one is a prefix of the other, we try to replay the
    // difference between `ours[start]` and `theirs[start]`.
    // If the diff fails, we pop the frame and replay it as a whole later
    if(start < shortest) {
      if(auto d = diff(theirs[start], ours[start]); d) {
        replay(*d, target);
        start++;
      } else {
        target->pop(1);
      }
    }
    
    // replay all the other frames
    for(size_t i = start; i < ours.size(); i++) {
      target->push();
      replay(ours[i], target); 
    }
  }

  void module::replay(module const&from, replay_target_t *target) const 
  {
    _impl->replay(from, target);
  }
  
  static term resolved(
    module const& m, term t, 
    support::set<variable> const& pending, bool *recursive, 
    immer::set<variable> hidden
  );
  
  static std::vector<term> resolved(
    module const& m, std::vector<term> const &ts, 
    support::set<variable> const& pending, bool *recursive, 
    immer::set<variable> hidden
  ) {
    std::vector<term> res;
    for(term t : ts)
      res.push_back(resolved(m, t, pending, recursive, hidden));
    return res;
  }

  static 
  term resolved(
    module const& m, term t, 
    support::set<variable> const& pending, bool *recursive, 
    immer::set<variable> hidden
  ) {
    return support::match(t)(
      [&](error v)         { return v; },
      [&](type_type v)     { return v; },
      [&](inferred_type v) { return v; },
      [&](integer_type v)  { return v; },
      [&](real_type v)     { return v; },
      [&](boolean_type v)  { return v; },
      [&](integer v)       { return v; },
      [&](real v)          { return v; },
      [&](boolean v)       { return v; },
      [&](object v)        { return v; },
      [&](variable x) -> term {
        if(hidden.count(x))
          return x;

        if(pending.contains(x))
          *recursive = true;

        if(auto obj = m.lookup(x); obj) {
          if(recursive && *recursive)
            return object(obj->lookup().unlocked()); // break reference cycle
          return *obj;
        }
        
        return x;
      },
      [&]<any_of<exists,forall,lambda> T>(T, auto const& decls, term body) {
        std::vector<decl> rdecls;
        for(decl d : decls) {
          rdecls.push_back(decl{
            d.name, resolved(m, d.type, {}, nullptr, hidden)
          });
          hidden = hidden.insert(d.name);
        }
        
        return T(rdecls, resolved(m, body, pending, recursive, hidden));
      },
      [&]<typename T>(T, auto const &...args) {
        return T(resolved(m, args, pending, recursive, hidden)...);
      }
    );
  }

  term module::resolved(term t) const {
    return logic::resolved(*this, t, {}, nullptr, {});
  }

  void module::resolve(scope s) {
    std::vector<object> objs;
    for(auto p : _impl->pending)
      objs.push_back(object(p));
    
    if(s == scope::recursive)
      adopt(objs, scope::linear);

    auto pending = _impl->pending;
    _impl->pending = {};

    bool recursive = false;
    support::set<variable> pendingvars;
    for(auto p : pending)
      pendingvars.insert(p->name);

    for(auto p : pending) {
      p->type = resolved(p->type);
      if(p->value) {
        *p->value = 
          logic::resolved(*this, *p->value, pendingvars, &recursive, {});

        p->type = support::match(p->type)(
          [&](inferred_type) {
            return type_of(*p->value);
          },
          [&](function_type t, auto const& params, term range) {
            if(!cast<inferred_type>(range))
              return t;
            auto func = cast<lambda>(p->value);
            if(!func)
              return t;

            module m;
            m.import(*this);
            for(decl d : func->vars())
              m.declare(d);

            return function_type(params, type_of(m.resolved(func->body())));
          },
          [&](auto t) { return t; }
        );
      }
    }

    if(s == scope::recursive && recursive)
      _impl->stack = _impl->stack.update(_impl->stack.size() - 1, [](auto top){
        top.sccs = top.sccs.update(top.sccs.size() - 1, [](auto scc) {
          scc.recursive = true;
          return scc;
        });
        return top;
      });
    else
      adopt(objs, scope::linear);
  }

  module module::resolved(scope s) const {
    module m = *this;
    m.resolve(s);
    return m;
  }

}

