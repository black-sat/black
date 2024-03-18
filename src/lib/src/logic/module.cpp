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
#include <immer/map.hpp>
#include <immer/set.hpp>

namespace black::logic {

  struct module::_impl_t 
  {
    alphabet *sigma;
    immer::vector<module> imports;
    immer::map<label, std::shared_ptr<struct lookup const>> lookups;
    immer::vector<std::shared_ptr<struct lookup>> pending;
    immer::vector<term> reqs;

    _impl_t(alphabet *sigma) : sigma{sigma} { }

    bool operator==(_impl_t const&) const = default;
  };

  module::module(alphabet *sigma) 
    : _impl{std::make_unique<_impl_t>(sigma)} { } 

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
    _impl->imports = _impl->imports.push_back(m.resolved());
  }

  object module::declare(decl d, resolution r) {
    auto ptr = std::make_shared<struct lookup>(d);
    _impl->pending = _impl->pending.push_back(ptr);
    
    if(r == resolution::immediate)
      resolve();

    return _impl->sigma->object(ptr.get());
  }
  
  object module::define(def d, resolution r) {
    auto ptr = std::make_shared<struct lookup>(d);
    _impl->pending = _impl->pending.push_back(ptr);
    
    if(r == resolution::immediate)
      resolve();

    return _impl->sigma->object(ptr.get());
  }

  object module::define(function_def f, resolution r) {
    std::vector<term> argtypes;
    for(decl d : f.parameters)
      argtypes.push_back(d.type);

    auto type = function_type(argtypes, f.range);
    auto body = lambda(f.parameters, f.body);

    return define(def{f.name, type, body}, r);
  }
  
  std::optional<object> module::lookup(label s) const {
    if(auto p = _impl->lookups.find(s); p)
      return _impl->sigma->object(p->get());
    
    for(auto imported : _impl->imports)
      if(auto result = imported.lookup(s); result)
        return result;
    
    return {};
  }

  void module::require(term req) {
    _impl->reqs = _impl->reqs.push_back(req);
  }

  alphabet *module::sigma() const {
    return _impl->sigma;
  }
  
  std::vector<module> module::imports() const {
    return std::vector<module>{_impl->imports.begin(), _impl->imports.end()};
  }
  
  std::vector<object> module::objects() const {
    return 
      std::views::all(_impl->lookups) | 
      std::views::transform([&](auto v) { 
        return _impl->sigma->object(v.second.get());
      }) |
      std::ranges::to<std::vector>();
  }

  std::vector<term> module::requirements() const {
    return std::vector<term>{_impl->reqs.begin(), _impl->reqs.end()};
  }

  static term resolved(module const& m, term t, immer::set<label> hidden);
  
  static std::vector<term> resolved(
    module const& m, std::vector<term> const &ts, immer::set<label> hidden
  ) {
    std::vector<term> res;
    for(term t : ts)
      res.push_back(resolved(m, t, hidden));
    return res;
  }

  static 
  term resolved(module const& m, term t, immer::set<label> hidden = {}) {
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
      [&](variable x, label name) -> term {
        if(hidden.count(name))
          return x;

        if(auto lookup = m.lookup(name); lookup)
          return *lookup;
        
        return x;
      },
      [&]<any_of<exists,forall,lambda> T>(T, auto const& decls, term body) {
        std::vector<decl> rdecls;
        for(decl d : decls) {
          rdecls.push_back(decl{d.name, resolved(m, d.type, hidden)});
          hidden = hidden.insert(d.name);
        }
        
        return T(rdecls, resolved(m, body, hidden));
      },
      [&]<typename T>(T, auto const &...args) {
        return T(resolved(m, args, hidden)...);
      }
    );
  }

  term module::resolved(term t) const {
    return logic::resolved(*this, t);
  }

  void module::resolve() {
    for(auto p : _impl->pending)
      _impl->lookups = _impl->lookups.insert({p->name, p});

    auto pending = _impl->pending;
    _impl->pending = {};

    for(auto p : pending) {
      p->type = resolved(p->type);
      if(p->value) {
        *p->value = resolved(*p->value);
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

            module m(_impl->sigma);
            m.import(*this);
            for(decl d : func->vars())
              m.declare(d);
            
            return function_type(params, type_of(m.resolved(func->body())));
          },
          [&](auto t) { return t; }
        );
      }
    }
  }

  module module::resolved() const {
    module m = *this;
    m.resolve();
    return m;
  }

}

