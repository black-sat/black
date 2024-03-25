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
#include <black/support/private>
#include <black/logic>

#include <algorithm>
#include <unordered_set>

namespace black::logic {

  namespace persistent = support::persistent;

  //
  // This is the internal layout of a `module` object.
  //
  // The entities are stored in a stack of frames, each of which contains:
  // 1. a vector of imported modules
  // 2. a vector of roots
  // 3. a map from `variable` to `entity const *`, for the name lookup.
  // 4. a vector of required terms.
  //
  // Each root contains a vector of lookups and is marked recursive or not.
  //
  // Pending entities are stored separately, outside of the stack.
  //
  struct module::impl_t
  {
    struct frame_t {
      persistent::vector<module> imports;
      persistent::vector<std::shared_ptr<root const>> roots;
      persistent::map<variable, entity const*> scope;
      persistent::vector<term> reqs;

      bool operator==(frame_t const&) const = default;
    };

    persistent::vector<frame_t> _stack = { frame_t{} };
    std::vector<support::boxed<entity>> _pending;

    impl_t() = default;

    impl_t(impl_t const&) = default;
    impl_t(impl_t &&) = default;
    
    impl_t &operator=(impl_t const&) = default;
    impl_t &operator=(impl_t &&) = default;

    bool operator==(impl_t const&) const = default;

    term resolved(
      term t, std::unordered_set<variable> const& pending, recursion *mode, 
      persistent::set<variable> hidden
    ) const;

    std::vector<term> resolved(
      std::vector<term> const &ts, 
      std::unordered_set<variable> const& pending, recursion *mode, 
      persistent::set<variable> hidden
    ) const;

    term infer(entity const *p);

    std::optional<frame_t>
    diff(frame_t const& ours, frame_t const& theirs) const;

    void replay(frame_t const& f, replay_target_t *target) const;
    
    void import(module m);
    object declare(decl d, resolution r);
    object define(def d, resolution r);
    void adopt(std::shared_ptr<root const> rs);
    std::optional<object> lookup(variable x) const;
    std::shared_ptr<root const> resolve(recursion s);
    void require(term req);
    void push();
    void pop(size_t n);
    void replay(module from, replay_target_t *target) const;
  };

  module::module() = default;

  module::module(module const&) = default;

  module::module(module &&) = default;

  module::~module() = default;

  module &module::operator=(module const&) = default;

  module &module::operator=(module &&) = default;

  bool module::operator==(module const&) const = default;

  void module::impl_t::import(module m) {
    m.resolve();
    _stack.update(_stack.size() - 1, [&](auto top) {
      top.imports.push_back(m);
      return top;
    });
  }

  void module::import(module m) {
    _impl->get()->import(m);
  }

  object module::impl_t::declare(decl d, resolution r) {
    auto e = support::boxed(entity{d});
    object obj{e.get()};
    _pending.push_back(std::move(e));
    
    if(r == resolution::immediate)
      resolve(recursion::forbidden);

    return obj;
  }

  object module::declare(decl d, resolution r) {
    return _impl->get()->declare(d, r);
  }

  object module::declare(variable name, term type, resolution r) {
    return declare(decl{name, type}, r);
  }
  
  object module::impl_t::define(def d, resolution r) {
    auto e = support::boxed(entity{d});
    object obj{e.get()};
    _pending.push_back(std::move(e));
    
    if(r == resolution::immediate)
      resolve(recursion::forbidden);

    return obj;
  }

  object module::define(def d, resolution r) {
    return _impl->get()->define(d, r);
  }

  object module::define(variable name, term type, term value, resolution r) {
    return define(def{name, type, value}, r);
  }

  object module::define(variable name, term value, resolution r) {
    return define(def{name, value}, r);
  }

  object module::define(function_def f, resolution r) {
    std::vector<term> argtypes;
    for(decl d : f.parameters)
      argtypes.push_back(d.type);

    auto type = function_type(argtypes, f.range);
    auto body = lambda(f.parameters, f.body);

    return define(def{f.name, type, body}, r);
  }

  object module::define(
    variable name, std::vector<decl> parameters,
    term range, term body, resolution r
  ) {
    return define(function_def{name, std::move(parameters), range, body}, r);
  }

  object module::define(
    variable name, std::vector<decl> parameters,
    term body, resolution r
  ) {
    return define(function_def{name, std::move(parameters), body}, r);
  }

  void module::impl_t::adopt(std::shared_ptr<root const> r) {
    if(!r)
      return;

    _stack.update(_stack.size() - 1, [&](auto top) {
      top.roots.push_back(r);
      for(auto const& e : r->entities)
        top.scope.set(e->name, e.get());
      return top;
    });
  }

  void module::adopt(std::shared_ptr<root const> r) {
    _impl->get()->adopt(r);
  }

  void module::adopt(object obj) {
    if(auto r = obj.entity()->root.lock(); r)
      adopt(r);
  }

  std::optional<object> module::impl_t::lookup(variable s) const 
  {
    for(auto it = _stack.rbegin(); it != _stack.rend(); it++)
      if(auto p = it->scope.find(s); p)
        return object(*p);
    
    for(auto it = _stack.rbegin(); it != _stack.rend(); it++) 
      for(auto im = it->imports.rbegin(); im != it->imports.rend(); im++)
        if(auto result = im->lookup(s); result)
          return result;   
    
    return {};
  }

  std::optional<object> module::lookup(variable s) const {
    return _impl->get()->lookup(s);
  }

  void module::impl_t::require(term req) {
    black_assert(!_stack.empty());
    _stack.update(_stack.size() - 1, [&](auto top) {
      top.reqs.push_back(req);
      return top;
    });
  }

  void module::require(term req) {
    _impl->get()->require(req);
  }

  void module::impl_t::push() {
    _stack.push_back({});
  }

  void module::push() {
    _impl->get()->push();
  }

  void module::impl_t::pop(size_t n) {
    black_assert(!_stack.empty());
    _stack.take(_stack.size() - n);
    if(_stack.empty())
      _stack.push_back({});
  }

  void module::pop(size_t n) {
    _impl->get()->pop(n);
  }

  auto module::impl_t::diff(frame_t const&inner, frame_t const&outer) const 
    -> std::optional<frame_t>
  {
    frame_t result;
    
    // check vectors sizes
    if(inner.imports.size() > outer.imports.size())
      return {};
    if(inner.roots.size() > outer.roots.size())
      return {};
    if(inner.reqs.size() > outer.reqs.size())
      return {};

    // check that inner is a prefix
    for(size_t i = 0; i < inner.imports.size(); i++)
      if(inner.imports[i] != outer.imports[i])
        return {};
    
    for(size_t i = 0; i < inner.roots.size(); i++)
      if(inner.roots[i] != outer.roots[i])
        return {};
    
    for(size_t i = 0; i < inner.reqs.size(); i++)
      if(inner.reqs[i] != outer.reqs[i])
        return {};
    
    // collect the additional one from outer
    for(size_t i = inner.imports.size(); i < outer.imports.size(); i++)
      result.imports.push_back(outer.imports[i]);
    
    for(size_t i = inner.roots.size(); i < outer.roots.size(); i++)
      result.roots.push_back(outer.roots[i]);
    
    for(size_t i = inner.reqs.size(); i < outer.reqs.size(); i++)
      result.reqs.push_back(outer.reqs[i]);

    return result;
  }
    
  void 
  module::impl_t::replay(frame_t const& f, replay_target_t *target) const {
    for(module m : f.imports)
      target->import(std::move(m));
    
    for(auto root : f.roots)
      target->adopt(root);
    
    for(term req : f.reqs)
      target->require(req);
  }

  //
  // Here we compute and issue the operations to replay from `from`.
  // 1. We compute a "longest common prefix" of our two stacks
  // 2. We pop from `from` the least we need to make it a subset of `*this`
  // 3. We replay the additional material
  //
  void 
  module::impl_t::replay(module from, replay_target_t *target) const {
    auto ours = _stack;
    auto theirs = from._impl->get()->_stack;
    size_t shortest = std::min(ours.size(), theirs.size());

    // 'start' is the index of the first different frame in `ours` and `theirs`
    // after the longest common prefix
    // if `start == shortest` one of the two is a strict subset of the other
    size_t start = 0;
    for(size_t n = shortest; n > 0; n--)
      if(ours.immutable().take(n) == theirs.immutable().take(n))
        start = n;

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

  void module::_replay(module from, replay_target_t *target) const {
    _impl->get()->replay(std::move(from), target);
  }
  
  std::vector<term> module::impl_t::resolved(
    std::vector<term> const &ts, 
    std::unordered_set<variable> const& pending, recursion *mode, 
    persistent::set<variable> hidden
  ) const {
    std::vector<term> res;
    for(term t : ts)
      res.push_back(resolved(t, pending, mode, hidden));
    return res;
  }

  //
  // Main recursive implementation of the name resolution in terms. As a
  // secondary outcome we set `*mode` to recursion::allowed if any variable from
  // `pending` is mentioned.
  //
  term module::impl_t::resolved(
    term t, std::unordered_set<variable> const& pending, recursion *mode, 
    persistent::set<variable> hidden
  ) const {
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
          *mode = recursion::allowed;

        if(auto obj = lookup(x); obj)
          return *obj;
        
        return x;
      },
      [&]<any_of<exists,forall,lambda> T>(T, auto const& decls, term body) {
        std::vector<decl> rdecls;
        for(decl d : decls) {
          rdecls.push_back(decl{
            d.name, resolved(d.type, {}, nullptr, hidden)
          });
          hidden.insert(d.name);
        }
        
        return T(rdecls, resolved(body, pending, mode, hidden));
      },
      [&]<typename T>(T, auto const &...args) {
        return T(resolved(args, pending, mode, hidden)...);
      }
    );
  }

  term module::resolved(term t) const {
    return _impl->get()->resolved(t, {}, nullptr, {});
  }

  term module::impl_t::infer(entity const *p) {
    return support::match(p->type)(
      [&](inferred_type) {
        return type_of(*p->value);
      },
      [&](function_type t, auto const& params, term range) {
        if(!cast<inferred_type>(range))
          return t;
        auto func = cast<lambda>(p->value);
        if(!func)
          return t;

        impl_t nested = *this;
        for(decl d : func->vars())
          nested.declare(d, resolution::immediate);

        return function_type(
          params, 
          type_of(nested.resolved(func->body(), {}, nullptr, {}))
        );
      },
      [&](auto t) { return t; }
    );
  }

  //
  // Main name resolution procedure.
  //
  std::shared_ptr<root const> module::impl_t::resolve(recursion r) {
    // at first, create the new root and reset the pending vector in the module,
    // and we collect things from the vector while we move it.
    auto root = std::make_shared<struct root>();
    std::vector<entity *> lookups;
    std::unordered_set<variable> names;
    for(auto pending = std::move(_pending); auto & e : pending) {
      lookups.push_back(e.get());
      e->root = root;
      names.insert(e->name);
      root->entities.push_back(std::move(e).release());
    }
    
    // in recursive mode, the pending root is adopted already so its lookups
    // will be visible to name lookup from now on. Note that `root.mode` is
    // still `recursion::forbidden` for now but it will be corrected at the end
    // if needed.
    if(r == recursion::allowed)
      adopt(root);

    recursion mode = recursion::forbidden;
    // for each pending entity:
    for(auto p : lookups) {
      // 1. we resolve the type (recursion in types is not supported)
      p->type = resolved(p->type, {}, nullptr, {});
      
      // 2. if a definition, we resolve the value, detecting recursion
      if(p->value) {
        *p->value = resolved(*p->value, names, &mode, {});

        // 3. we infer the type if needed
        p->type = infer(p);
      }
    }

    // finally, if recursion is allowed, we mark the root as recursive or not as
    // detected, otherwise we adopt the root now because we didn't before.
    if(mode == recursion::allowed)
      root->mode = mode;
    else
      adopt(root);

    return root;
  }

  std::shared_ptr<root const> module::resolve(recursion r) {
    return _impl->get()->resolve(r);
  }

}

