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

#include <ranges>

namespace black::logic {

  struct decl_inner_t {
    variable name;
    term type;
    std::optional<term> def;
  };

  struct module::_impl_t 
  {
    alphabet *sigma;
    immer::vector<module> imports;
    immer::map<variable, decl_inner_t> decls;
  };

  module::module(alphabet *sigma) 
    : _impl{std::make_unique<_impl_t>(_impl_t{sigma, {}, {}})} { } 

  module::module(module const& other)
    : _impl{std::make_unique<_impl_t>(_impl_t{*other._impl})} { }

  module::~module() = default;

  module &module::operator=(module const& other) {
    *_impl = *other._impl;
    return *this;
  }

  void module::import(module m) {
    _impl->imports = _impl->imports.push_back(std::move(m));
  }

  variable module::declare(variable x, term ty) {
    _impl->decls = _impl->decls.insert({x, decl_inner_t{x, ty, {}}});
    return x;
  }

  variable module::declare(binding b) {
    return declare(b.name, b.target);
  }

  variable 
  module::declare(variable x, std::vector<term> const &params, term range) {
    return declare(x, function_type(params, range));
  }

  void module::declare(std::vector<binding> const& binds) {
    for(binding b : binds)
      declare(b);
  }

  variable module::declare(label s, term type) {
    return declare(type.sigma()->variable(s), type);
  }

  variable module::declare(label s, std::vector<term> const &params, term range)
  {
    return declare(range.sigma()->variable(s), params, range);
  }

  variable module::define(variable x, term type, term def) {
    _impl->decls = _impl->decls.insert({x, decl_inner_t{x, type, def}});
    return x;
  }

  variable module::define(def d) {
    return define(d.name, d.type, d.def);
  }
  
  variable module::define(
    variable x, std::vector<binding> const &params, term range, term body
  ) {
    std::vector<term> paramtypes;
    for(auto p : params)
      paramtypes.push_back(p.target);
    
    return define(x, function_type(paramtypes, range), lambda(params, body));
  }
  
  void module::define(std::vector<def> const& defs) 
  {
    for(auto def : defs)
      define(def);
  }
    
  variable module::define(label s, term type, term def) {
    return define(type.sigma()->variable(s), type, def);
  }

  variable module::define(
    label s, std::vector<binding> const &params, term range, term body
  ) {
    return define(range.sigma()->variable(s), params, range, body);
  }

  alphabet *module::sigma() const {
    return _impl->sigma;
  }
  
  std::optional<decl> module::lookup(variable s) const {
    if(auto it = _impl->decls.find(s); it)
      return decl{this, it->name, it->type, it->def};
    
    for(auto imported : _impl->imports)
      if(auto result = imported.lookup(s); result)
        return result;
    
    return {};
  }

  term module::type_of(term t) const {
    using support::match;

    alphabet *sigma = t.sigma();

    return match(t)(
      [&](error e)       { return e; },
      [&](type_type)     { return sigma->type_type(); },
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
        auto d = lookup(x);
        if(!d)
          return error(x, "Use of undeclared variable");
        return d->scope->evaluate(d->type);
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
      [&](quantifier auto, auto const& binds, term body) -> term {
        module env(sigma);
        env.declare(binds);

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
      [&](lambda, std::vector<binding> const& binds, term body) -> term {
        std::vector<term> argtypes;
        for(binding b : binds)
          argtypes.push_back(b.target);
        
        module env(sigma);
        env.import(*this);
        env.declare(binds);

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
      [&](integer_type v)  { return v; },
      [&](real_type v)     { return v; },
      [&](boolean_type v)  { return v; },
      [&](function_type v) { return v; },
      [&](integer v)       { return v; },
      [&](real v)          { return v; },
      [&](boolean v)       { return v; },
      [&](lambda v)        { return v; },
      [&](variable x) -> term {
        auto d = lookup(x);
        if(!d || !d->def)
          return x;
        
        return d->scope->evaluate(*d->def);
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
          env.define(f->vars()[i].name, f->vars()[i].target, eargs[i]);

        return env.evaluate(f->body());
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
      [&](relational auto op, term left, term right) -> term {
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

