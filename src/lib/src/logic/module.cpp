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
            
            return function_type(params, m.type_of(m.resolved(func->body())));
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

  term module::type_of(term t) const {
    using support::match;

    alphabet *sigma = t.sigma();

    return match(t)(
      [&](error e)       { return e; },
      [&](type_type)     { return sigma->type_type(); },
      [&](inferred_type) { return sigma->type_type(); },
      [&](integer_type)  { return sigma->type_type(); },
      [&](real_type)     { return sigma->type_type(); },
      [&](boolean_type)  { return sigma->type_type(); },
      [&](function_type) { return sigma->type_type(); },
      [&](integer)       { return sigma->integer_type(); },
      [&](real)          { return sigma->real_type(); },
      [&](boolean)       { return sigma->boolean_type(); },
      [&](equal)         { return sigma->boolean_type(); },
      [&](distinct)      { return sigma->boolean_type(); },
      [&](type_cast c)   { return c.target(); },
      [&](variable x) -> term {
        return error(x, "use of unbound free variable");
      },
      [&](object, auto lookup) {
        return lookup->type;
      },
      [&](atom a, term head, auto const& args) -> term {
        auto fty = cast<function_type>(type_of(head));
        if(!fty)
          return error(a, "calling a non-function");
          
        if(args.size() != fty->parameters().size()) 
          return error(a, "argument number mismatch in function call");

        for(size_t i = 0; i < args.size(); i++) {
          auto type = type_of(args[i]);

          if(type != fty->parameters()[i])
            return error(a, "type mismatch in function call");
        }

        return fty->range();
      },
      [&](quantifier auto, auto const& decls, term body) -> term {
        module env(sigma);
        for(decl d : decls)
          env.declare(d);

        term bodyty = env.type_of(body);
        if(!cast<boolean_type>(bodyty))
          return error(body, "quantified terms must be boolean");
        
        return sigma->boolean_type();
      },
      [&](any_of<negation, implication> auto c, auto ...args) -> term {
        term argtypes[] = {type_of(args)...};
        for(auto ty : argtypes) {
          if(!cast<boolean_type>(ty))
            return 
              error(c, "connectives can be applied only to boolean terms");
        }

        return sigma->boolean_type();
      },
      [&](any_of<conjunction, disjunction> auto c, auto const& args) -> term {
        for(term arg : args) 
          if(!cast<boolean_type>(type_of(arg)))
            return 
              error(c, "connectives can be applied only to boolean terms");

        return sigma->boolean_type();
      },
      [&](ite f, term guard, term iftrue, term iffalse) -> term {
        if(!cast<boolean_type>(type_of(guard)))
          return error(f, "the guard of an `ite` expression must be boolean");

        auto truety = type_of(iftrue);
        auto falsety = type_of(iffalse);
        if(truety != falsety)
          return 
            error(f,
              "the two cases of an `ite` expression must have the same type"
            );

        return truety;
      },
      [&](lambda, auto const& decls, term body) -> term {
        std::vector<term> argtypes;
        for(decl d : decls)
          argtypes.push_back(d.type);
        
        module env(sigma);
        env.import(*this);
        for(decl d : decls)
          env.declare(d);

        auto bodyty = env.type_of(body);
        return function_type(std::move(argtypes), bodyty);
      },
      // case_of...
      [&](temporal auto tm, auto ...args) -> term {
        term argtypes[] = {type_of(args)...};
        for(auto ty : argtypes)
          if(!cast<boolean_type>(ty))
            return 
              error(tm,
                "temporal operators can be applied only to boolean terms"
              );

        return sigma->boolean_type();
      },
      [&](minus m, term arg) -> term {
        term type = type_of(arg);
        
        if(
          bool(type != sigma->integer_type()) && 
          bool(type != sigma->real_type())
        )
          return 
            error(m, "arithmetic operators only work on integers or reals");
        
        return type;
      },
      [&](arithmetic auto a, term left, term right) -> term {
        term type1 = type_of(left);
        term type2 = type_of(right);
        
        if(
          bool(type1 != sigma->integer_type()) && 
          bool(type1 != sigma->real_type())
        )
          return 
            error(a, 
              "left side of arithmetic operator must be integer or real"
            );
        
        if(
          bool(type2 != sigma->integer_type()) && 
          bool(type2 != sigma->real_type())
        )
          return 
            error(a, 
              "right side of arithmetic operator must be integer or real"
            );

        if(type1 != type2)
          return 
            error(a, "arithmetic operators can only be applied to equal types");
        
        return type1;
      },
      [&](relational auto r, term left, term right) -> term {
        term type1 = type_of(left);
        term type2 = type_of(right);
        
        if(
          bool(type1 != sigma->integer_type()) && 
          bool(type1 != sigma->real_type())
        )
          return 
            error(r, 
              "left side of relational operator must be integer or real"
            );
        
        if(
          bool(type2 != sigma->integer_type()) && 
          bool(type2 != sigma->real_type())
        )
          return 
            error(r, 
              "right side of relational operator must be integer or real"
            );

        if(type1 != type2)
          return 
            error(r, "relational operators can only be applied to equal types");
        
        return sigma->boolean_type();
      }
    );
  }

  term module::evaluate(term t) const {
    using support::match;

    alphabet *sigma = t.sigma();

    return match(t)(
      [&](type_type v)     { return v; },
      [&](inferred_type v) { return v; },
      [&](integer_type v)  { return v; },
      [&](real_type v)     { return v; },
      [&](boolean_type v)  { return v; },
      [&](function_type v) { return v; },
      [&](integer v)       { return v; },
      [&](real v)          { return v; },
      [&](boolean v)       { return v; },
      [&](lambda v)        { return v; },
      [&](variable x)      { return x; },
      [&](object x, auto lookup) -> term {
        if(lookup->value)
          return evaluate(*lookup->value);
        return x;
      },
      //[&](type_cast c)   { return c.target(); },
      [&](atom, term head, auto const& args) -> term {

        term ehead = evaluate(head);
        std::vector<term> eargs = evaluate(args);

        auto f = cast<lambda>(ehead);
        if(!f || f->vars().size() != args.size())
          return atom(ehead, eargs);
        
        module env(sigma);
        env.import(*this);

        for(size_t i = 0; i < args.size(); i++)
          env.define({f->vars()[i].name, f->vars()[i].type, eargs[i]});

        return env.evaluate(env.resolved(f->body()));
      },
      // equal, distinct
      [&]<quantifier T>(T, auto const& binds, term body) {
        return T(binds, evaluate(body));
      },
      [&]<temporal T>(T, auto ...args) {
        return T(evaluate(args)...);
      },
      [&](negation, term arg) -> term {
        term earg = evaluate(arg);
        if(auto v = cast<boolean>(earg); v)
          return sigma->boolean(!v->value());
        
        return negation(earg);
      },
      [&](conjunction, auto const& args) -> term {
        std::vector<term> eargs = evaluate(args);
        bool result = true;
        
        for(auto earg : eargs) {
          if(auto v = cast<boolean>(earg); v)
            result = result && v->value();
          return conjunction(eargs);
        }

        return sigma->boolean(result);
      },
      [&](disjunction, auto const& args) -> term {
        std::vector<term> eargs = evaluate(args);
        bool result = false;
        
        for(auto earg : eargs) {
          if(auto v = cast<boolean>(earg); v)
            result = result || v->value();
          return conjunction(eargs);
        }

        return sigma->boolean(result);
      },
      [&](implication, term left, term right) -> term {
        term eleft = evaluate(left);
        term eright = evaluate(right);
        
        auto vl = cast<boolean>(eleft);
        auto vr = cast<boolean>(eright);

        if(!vl || !vr)
          return implication(eleft, eright);
        
        return sigma->boolean(!vl->value() || vr->value());
      },
      [&](ite, term guard, term iftrue, term iffalse) -> term {
        term eguard = evaluate(guard);
        term etrue = evaluate(iftrue);
        term efalse = evaluate(iffalse);

        auto guardv = cast<boolean>(eguard);
        if(!guardv)
          return ite(eguard, etrue, efalse);

        if(guardv->value())
          return etrue;
        return efalse;
      },
      [&](minus, term arg) -> term {
        term earg = evaluate(arg);
        
        return match(earg)(
          [&](integer, auto value) {
            return sigma->integer(-value);
          },
          [&](real, auto value) {
            return sigma->real(-value);
          },
          [&](auto) {
            return minus(earg);
          }
        );
      },
      [&]<arithmetic T>(T op, term left, term right) -> term {
        term eleft = evaluate(left);
        term eright = evaluate(right);

        auto ilv = cast<integer>(eleft);
        auto ilr = cast<integer>(eright);
        if(ilv && ilr)
          return sigma->integer(
            match(op)(
              [&](sum) { return ilv->value() + ilr->value(); },
              [&](product) { return ilv->value() * ilr->value(); },
              [&](difference) { return ilv->value() - ilr->value(); },
              [&](division) { return ilv->value() / ilr->value(); }
            )
          );
        
        auto rlv = cast<real>(eleft);
        auto rlr = cast<real>(eright);
        if(rlv && rlr)
          return sigma->real(
            match(op)(
              [&](sum) { return rlv->value() + rlr->value(); },
              [&](product) { return rlv->value() * rlr->value(); },
              [&](difference) { return rlv->value() - rlr->value(); },
              [&](division) { return rlv->value() / rlr->value(); }
            )
          );

        return T(eleft, eright);
      },
      [&]<relational T>(T op, term left, term right) -> term {
        term eleft = evaluate(left);
        term eright = evaluate(right);

        auto ilv = cast<integer>(eleft);
        auto ilr = cast<integer>(eright);
        if(ilv && ilr)
          return sigma->boolean(
            match(op)(
              [&](less_than) { return ilv->value() < ilr->value(); },
              [&](less_than_eq) { return ilv->value() <= ilr->value(); },
              [&](greater_than) { return ilv->value() > ilr->value(); },
              [&](greater_than_eq) { return ilv->value() >= ilr->value(); }
            )
          );
        
        auto rlv = cast<real>(eleft);
        auto rlr = cast<real>(eright);
        if(rlv && rlr)
          return sigma->boolean(
            match(op)(
              [&](less_than) { return rlv->value() < rlr->value(); },
              [&](less_than_eq) { return rlv->value() <= rlr->value(); },
              [&](greater_than) { return rlv->value() > rlr->value(); },
              [&](greater_than_eq) { return rlv->value() >= rlr->value(); }
            )
          );

        return T(eleft, eright);
      }
    );
  }

  std::vector<term> module::type_of(std::vector<term> const& ts) const {
    std::vector<term> result;
    for(term t : ts)
      result.push_back(type_of(t));
    return result;
  }
  
  std::vector<term> module::evaluate(std::vector<term> const& ts) const {
    std::vector<term> result;
    for(term t : ts)
      result.push_back(evaluate(t));
    return result;
  }

}

