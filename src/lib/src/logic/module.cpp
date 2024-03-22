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

namespace black::logic {

  namespace persistent = support::persistent;

  //
  // This is the internal layout of a `module` object.
  //
  // The entities are stored in a stack of frames, each of which contains:
  // 1. a vector of imported modules
  // 2. a vector of groups of lookups resolved together, called SCCs (although
  //    they are not really strongly connected, at the moment).
  // 3. a map from `variable` to `lookup`, for the name lookup.
  // 4. a vector of required terms.
  //
  // Each SCC is marked recursive or not, and contains a set of lookups (because
  // the order of declaration is irrelevant inside the single set).
  //
  // Pending declarations/definitions are stored separately, outside of the
  // _stack.
  //
  struct module::impl_t : support::pimpl_base<module, impl_t>
  {
    struct scc_t {
      bool recursive;
      persistent::set<std::shared_ptr<struct lookup const>> lookups;

      bool operator==(scc_t const&) const = default;
    };
    
    struct frame_t {
      persistent::vector<module> imports;
      persistent::vector<scc_t> sccs;
      persistent::map<variable, struct lookup const*> scope;
      persistent::vector<term> reqs;

      bool operator==(frame_t const&) const = default;
    };

    impl_t() : _stack{frame_t{}} { }

    bool operator==(impl_t const&) const = default;

    term resolved(
      term t, support::set<variable> const& pending, bool *recursive, 
      persistent::set<variable> hidden
    ) const;

    std::vector<term> resolved(
      std::vector<term> const &ts, 
      support::set<variable> const& pending, bool *recursive, 
      persistent::set<variable> hidden
    ) const;

    std::optional<frame_t>
    diff(frame_t const& ours, frame_t const& theirs) const;

    void replay(frame_t const& f, replay_target_t *target) const;
    
    void import(module m);
    object declare(decl d, resolution r);
    object define(def d, resolution r);
    void adopt(std::vector<object> const& objs, scope s);
    std::optional<object> lookup(variable x) const;
    void resolve(scope s);
    void require(term req);
    void push();
    void pop(size_t n);
    void replay(module const &from, replay_target_t *target) const;

    persistent::vector<frame_t> _stack;
    persistent::set<std::shared_ptr<struct lookup>> _pending;
  };

  module::module() : _impl{std::make_unique<impl_t>()} { } 

  module::module(std::unique_ptr<impl_t> ptr) : _impl{std::move(ptr)} { }

  module::module(module const& other) 
    : _impl{std::make_unique<impl_t>(*other._impl)} { }

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

  void module::impl_t::import(module m) {
    m.resolve();
    _stack.update(_stack.size() - 1, [&](auto top) {
      top.imports.push_back(m);
      return top;
    });
  }

  void module::import(module m) {
    _impl->import(m);
  }

  object module::impl_t::declare(decl d, resolution r) {
    auto ptr = std::make_shared<struct lookup>(d);
    _pending.insert(ptr);
    
    if(r == resolution::immediate)
      resolve(scope::linear);

    return object(ptr);
  }

  object module::declare(decl d, resolution r) {
    return _impl->declare(d, r);
  }

  object module::declare(variable name, term type, resolution r) {
    return declare(decl{name, type}, r);
  }
  
  object module::impl_t::define(def d, resolution r) {
    auto ptr = std::make_shared<struct lookup>(d);
    _pending.insert(ptr);
    
    if(r == resolution::immediate)
      resolve(scope::linear);

    return object(ptr);
  }

  object module::define(def d, resolution r) {
    return _impl->define(d, r);
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

  void module::impl_t::adopt(std::vector<object> const& objs, scope s) {
    impl_t::scc_t scc = {s == scope::recursive, {}};
    for(object obj : objs) {
      auto lu = obj.lookup().shared();
      scc.lookups.insert(lu);
    }
    _stack.update(_stack.size() - 1, [&](auto top) {
      top.sccs.push_back(scc);
      for(auto lu : scc.lookups)
        top.scope.set(lu->name, lu.get());
      return top;
    });
  }

  void module::adopt(std::vector<object> const& objs, scope s) {
    _impl->adopt(objs, s);
  }

  std::optional<object> module::impl_t::lookup(variable s) const 
  {
    for(auto it = _stack.rbegin(); it != _stack.rend(); it++)
      if(auto p = it->scope.find(s); p)
        return object((*p)->shared_from_this());
    
    for(auto it = _stack.rbegin(); it != _stack.rend(); it++) 
      for(auto im = it->imports.rbegin(); im != it->imports.rend(); im++)
        if(auto result = im->lookup(s); result)
          return result;   
    
    return {};
  }

  std::optional<object> module::lookup(variable s) const {
    return _impl->lookup(s);
  }

  void module::impl_t::require(term req) {
    black_assert(!_stack.empty());
    _stack.update(_stack.size() - 1, [&](auto top) {
      top.reqs.push_back(req);
      return top;
    });
  }

  void module::require(term req) {
    _impl->require(req);
  }

  void module::impl_t::push() {
    _stack.push_back({});
  }

  void module::push() {
    _impl->push();
  }

  void module::impl_t::pop(size_t n) {
    black_assert(!_stack.empty());
    _stack.take(_stack.size() - n);
    if(_stack.empty())
      _stack.push_back({});
  }

  void module::pop(size_t n) {
    _impl->pop(n);
  }

  auto module::impl_t::diff(frame_t const&inner, frame_t const&outer) const 
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
      result.imports.push_back(outer.imports[i]);
    
    for(size_t i = inner.sccs.size(); i < outer.sccs.size(); i++)
      result.sccs.push_back(outer.sccs[i]);
    
    for(size_t i = inner.reqs.size(); i < outer.reqs.size(); i++)
      result.reqs.push_back(outer.reqs[i]);

    return result;
  }
    
  void 
  module::impl_t::replay(frame_t const& f, replay_target_t *target) const {
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

  //
  // Here we compute and issue the operations to replay from `from`.
  // 1. We compute a "longest common prefix" of our two stacks
  // 2. We pop from `from` the least we need to make it a subset of `*this`
  // 3. We replay the additional material
  //
  void 
  module::impl_t::replay(module const&from, replay_target_t *target) const {
    auto ours = _stack;
    auto theirs = from._impl->_stack;
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

  void module::replay(module const&from, replay_target_t *target) const {
    _impl->replay(from, target);
  }
  
  std::vector<term> module::impl_t::resolved(
    std::vector<term> const &ts, 
    support::set<variable> const& pending, bool *recursive, 
    persistent::set<variable> hidden
  ) const {
    std::vector<term> res;
    for(term t : ts)
      res.push_back(resolved(t, pending, recursive, hidden));
    return res;
  }

  //
  // Main recursive implementation of the name resolution in terms. As a
  // secondary outcome we set `*recursive` to true if any variable from
  // `pending` is mentioned.
  //
  term module::impl_t::resolved(
    term t, support::set<variable> const& pending, bool *recursive, 
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
          *recursive = true;

        if(auto obj = lookup(x); obj) {
          if(recursive && *recursive)
            return obj->unlocked(); // break reference cycle
          return *obj;
        }
        
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
        
        return T(rdecls, resolved(body, pending, recursive, hidden));
      },
      [&]<typename T>(T, auto const &...args) {
        return T(resolved(args, pending, recursive, hidden)...);
      }
    );
  }

  term module::resolved(term t) const {
    return _impl->resolved(t, {}, nullptr, {});
  }

  //
  // Main name resolution procedure.
  //
  void module::impl_t::resolve(scope s) {
    // A collection of objects is made from the pending lookups
    std::vector<object> objs;
    for(auto p : _pending)
      objs.push_back(object(p));
    
    // in recursive mode, such objects are adopted already so they will be
    // visible to name lookup from now on. Note the scope::linear here, 
    // which will be corrected at the end if needed.
    if(s == scope::recursive)
      adopt(objs, scope::linear);

    // we save the pending lookups but reset the pending set of the module.
    auto pending = _pending;
    _pending = {};

    // we collect the names of the entities
    bool recursive = false;
    support::set<variable> pendingvars;
    for(auto p : pending)
      pendingvars.insert(p->name);

    // for each entity:
    for(auto p : pending) {
      // 1. we resolve the type (recursion in types is not supported)
      p->type = resolved(p->type, {}, nullptr, {});
      
      // 2. if a definition, we resolve the value, detecting recursion
      if(p->value) {
        *p->value = resolved(*p->value, pendingvars, &recursive, {});

        // 3. we replace `inferred_type` with the type of the resolved value
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

            module m = self();
            for(decl d : func->vars())
              m.declare(d);

            return function_type(params, type_of(m.resolved(func->body())));
          },
          [&](auto t) { return t; }
        );
      }
    }

    // if in recursive mode, and when recursion is actually detected, we mark
    // the last SCC as recursive
    if(s == scope::recursive) {
      if(recursive) {
        _stack.update(_stack.size() - 1, 
          [](auto top){
            top.sccs.update(top.sccs.size() - 1, [](auto scc) {
              scc.recursive = true;
              return scc;
            });
            return top;
          });
      }
    } else { 
      adopt(objs, scope::linear); // otherwise, we adopt the objects
    }
  }

  void module::resolve(scope s) {
    return _impl->resolve(s);
  }

}

