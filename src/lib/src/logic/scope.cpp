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
#include <black/io>
#include <black/logic>

namespace black::logic {

  scope::result<term> 
  scope::resolve(term t, support::set<symbol> const& shadow) const {
    // TODO: extract the recursion logic into a generic helper
    using support::match;

    alphabet *sigma = t.sigma();

    return match(t)(
      [&](type_type v)     { return v; },
      [&](integer_type v)  { return v; },
      [&](real_type v)     { return v; },
      [&](boolean_type v)  { return v; },
      [&](integer v)       { return v; },
      [&](real v)          { return v; },
      [&](boolean v)       { return v; },
      [&](variable v)      { return v; },
      [&](symbol s) -> result<term> {
        if(shadow.contains(s))
          return s;
        auto decl = lookup(s);
        if(!decl)
          return error("use of undeclared symbol");
        
        return sigma->variable(*decl);
      },
      [&](function_type, auto const & args, term range) -> result<term> {
        std::vector<term> resargs;
        for(auto arg : args) {
          auto res = resolve(arg, shadow);
          if(!res)
            return res;
          resargs.push_back(*res);
        }
        
        auto resrange = resolve(range, shadow);
        if(!resrange)
          return resrange;
        
        return atom(*resrange, resargs);
      },
      [&](atom, term head, auto const &args) -> result<term> {
        auto reshead = resolve(head, shadow);
        if(!reshead)
          return reshead;

        std::vector<term> resargs;
        for(auto arg : args) {
          auto res = resolve(arg, shadow);
          if(!res)
            return res;
          resargs.push_back(*res);
        }
        
        return atom(*reshead, resargs);
      },
      [&]<any_of<lambda, exists, forall> T>
      (T, auto const& bindings, term body) -> result<term> 
      {
        support::set<symbol> nest = shadow;
        for(binding bind : bindings)
          nest.insert(bind.name);

        auto res = resolve(body, nest);
        if(!res)
          return res;

        return T(bindings, *res);
      },
      [&]<any_of<conjunction, disjunction, equal, distinct> T>
      (T, auto const& args) -> result<term> 
      {
        std::vector<term> resargs;
        for(term arg : args) {
          auto res = resolve(arg, shadow);
          if(!res)
            return res;
          resargs.push_back(*res);
        }

        return T(std::move(resargs));
      },
      [&]<typename T>(T, auto ...args) -> result<term> {
        std::vector<result<term>> resargs = { resolve(args, shadow)... };
        for(auto res : resargs)
          if(!res)
            return res;

        return [&]<size_t ...Idx>(std::index_sequence<Idx...>) {
          return T(*resargs[Idx]...);
        }(std::make_index_sequence<sizeof...(args)>{});
      }
    );
  }

  scope::result<term> scope::type_of(term t) const {
    using support::match;

    alphabet *sigma = t.sigma();

    return match(t)(
      [&](type_type)     { return sigma->type_type(); },
      [&](integer_type)  { return sigma->type_type(); },
      [&](real_type)     { return sigma->type_type(); },
      [&](boolean_type)  { return sigma->type_type(); },
      [&](integer)       { return sigma->integer_type(); },
      [&](real)          { return sigma->real_type(); },
      [&](boolean)       { return sigma->boolean_type(); },
      [&](equal)         { return sigma->boolean_type(); },
      [&](distinct)      { return sigma->boolean_type(); },
      [&](type_cast c)   { return c.target(); },
      // `symbol` purposefully not handled here
      [&](variable, auto decl) {
        return decl->type();
      },
      [&](function_type, auto const& params, term range) -> result<term>
      { 
        for(term p : params) {
          result<bool> ist = is_type(p);
          if(!ist)
            return ist.error();
          if(!*ist)
            return error("type of function parameter is not a type");
        }
        result<bool> ist = is_type(range);
        if(!ist)
          return ist.error();
        if(!*ist)
          return error("function range is not a type");

        return sigma->type_type(); 
      },
      [&](atom, term head, auto const& args) -> result<term> {
        result<term> rfty = type_of(head);
        if(!rfty)
          return rfty;

        auto fty = cast<function_type>(rfty);
        if(!fty)
          return error("calling a non-function");
          
        if(args.size() != fty->parameters().size()) 
          return error("argument number mismatch in function call");

        for(size_t i = 0; i < args.size(); i++) {
          auto type = type_of(args[i]);
          if(!type)
            return type.error();

          if(*type != fty->parameters()[i])
            return error("type mismatch in function call");
        }

        return fty->range();
      },
      [&](quantifier auto, auto const& decls, term body) -> result<term>
      {
        module nest(this);
        nest.declare(decls);

        result<term> type = nest.type_of(body);
        if(!type)
          return type;
        
        if(!cast<boolean_type>(type))
          return error("quantified terms must be boolean");
        
        return sigma->boolean_type();
      },
      [&](any_of<negation, implication> auto, auto ...args) -> result<term> {
        result<term> argtypes[] = {type_of(args)...};
        for(auto ty : argtypes) {
          if(!ty)
            return ty;
          if(!cast<boolean_type>(ty))
            return 
              error("connectives can be applied only to boolean terms");
        }

        return sigma->boolean_type();
      },
      [&](any_of<conjunction, disjunction> auto, std::vector<term> const& args) 
        -> result<term> 
      {
        for(term arg : args) { 
          auto argty = type_of(arg);
          if(!argty)
            return argty;
          
          if(!cast<boolean_type>(argty))
            return 
              error("connectives can be applied only to boolean terms");
        }

        return sigma->boolean_type();
      },
      [&](ite, term guard, term iftrue, term iffalse) -> result<term> {
        auto guardty = type_of(guard);
        if(!guardty)
          return guardty;
        
        auto truety = type_of(iftrue);
        if(!truety)
          return truety;
        
        auto falsety = type_of(iffalse);
        if(!falsety)
          return falsety;

        if(!cast<boolean_type>(guardty))
          return error("the guard of an `ite` expression must be boolean");
        if(*truety != *falsety)
          return 
            error(
              "the two cases of an `ite` expression must have the same type"
            );

        return *truety;
      },
      [&](lambda, std::vector<binding> const& binds, term body) 
        -> result<term> 
      {
        std::vector<term> argtypes;
        for(binding b : binds)
          argtypes.push_back(b.target);
        
        module nest(this);
        nest.declare(binds);

        auto bodyty = nest.type_of(body);
        if(!bodyty)
          return bodyty;
        return function_type(std::move(argtypes), *bodyty);
      },
      // case_of...
      [&](temporal auto, auto ...args) -> result<term> {
        result<term> argtypes[] = {type_of(args)...};
        for(auto ty : argtypes) {
          if(!ty)
            return ty;
          if(!cast<boolean_type>(ty))
            return 
              error(
                "temporal operators can be applied only to boolean terms"
              );
        }

        return sigma->boolean_type();
      },
      [&](minus, term arg) -> result<term> {
        result<term> type = type_of(arg);
        if(!type)
          return type;
        
        if(type != sigma->integer_type() && type != sigma->real_type())
          return 
            error("arithmetic operations only work on integers or reals");
        
        return *type;
      },
      [&](arithmetic auto, term first, term second) -> result<term> {
        result<term> type1 = type_of(first);
        if(!type1)
          return type1;
        
        result<term> type2 = type_of(second);
        if(!type2)
          return type2;
        
        if(type1 != sigma->integer_type() && type1 != sigma->real_type())
          return 
            error("arithmetic operations only work on integers or reals");
        
        if(type2 != sigma->integer_type() && type1 != sigma->real_type())
          return 
            error("arithmetic operations only work on integers or reals");

        if(*type1 != *type2)
          return 
            error(
              "arithmetic operations can only be applied to equal types"
            );
        
        return *type1;
      },
      [&](relational auto, term left, term right) -> result<term> {
        result<term> type1 = type_of(left);
        if(!type1)
          return type1;
        
        result<term> type2 = type_of(right);
        if(!type2)
          return type2;
        
        if(type1 != sigma->integer_type() && type1 != sigma->real_type())
          return 
            error("relational operations only work on integers or reals");
        
        if(type2 != sigma->integer_type() && type1 != sigma->real_type())
          return 
            error("relational operations only work on integers or reals");

        if(*type1 != *type2)
          return 
            error(
              "relational operations can only be applied to equal types"
            );
        
        return sigma->boolean_type();
      }
    );
  }

  scope::result<term> scope::evaluate(term t) const {
    using support::match;

    alphabet *sigma = t.sigma();

    if(result<term> type = type_of(t); !type)
      return type;

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
      // `symbol` purposefully not handled here
      [&](variable, auto decl) -> result<term> {
        if(!decl->def())
          return error("cannot evaluate a declared variable");
        
        return evaluate(*decl->def());       
      },
      //[&](type_cast c)   { return c.target(); },
      [&](atom, term head, auto const& args) -> result<term> {
        auto h = evaluate(head);
        if(!h)
          return h;
        auto f = cast<lambda>(h);
        black_assert(f);

        if(f->vars().size() != args.size())
          return error("number of arguments mismatch in function call");
        
        module nest(this);

        for(size_t i = 0; i < args.size(); i++) {
          auto argv = evaluate(args[i]);
          if(!argv)
            return argv;

          nest.define(f->vars()[i].name, *argv);
        }

        return nest.evaluate(f->body());
      },
      [&](equal, auto const& args) { 
        for(size_t i = 1; i < args.size(); i++)
          if(args[i] != args[0])
            return sigma->boolean(false);
        return sigma->boolean(true);
      },
      [&](distinct, auto const& args) { 
        for(size_t i = 1; i < args.size(); i++)
          if(args[i] != args[0])
            return sigma->boolean(true);
        return sigma->boolean(false);
      },
      // quantifier...
      [&](temporal auto) -> result<term> {
        return error("cannot evaluate a temporal formula");
      },
      [&](negation, term arg) -> result<term> {
        auto v = cast<boolean>(evaluate(arg));
        if(!v)
          return v.error();
        
        return sigma->boolean(!v->value());
      },
      [&](conjunction, auto const& args) -> result<term> {
        bool result = true;
        for(auto arg : args) {
          auto v = cast<boolean>(evaluate(arg));
          if(!v)
            return v;
          result = result && v->value();
        }
        return sigma->boolean(result);
      },
      [&](disjunction, auto const& args) -> result<term> {
        bool result = true;
        for(auto arg : args) {
          auto v = cast<boolean>(evaluate(arg));
          if(!v)
            return v;
          result = result || v->value();
        }
        return sigma->boolean(result);
      },
      [&](implication, term left, term right) -> result<term> {
        auto vl = cast<boolean>(evaluate(left));
        if(!vl)
          return vl;
        auto vr = cast<boolean>(evaluate(right));
        if(!vr)
          return vr;
        
        return sigma->boolean(!vl->value() || vr->value());
      },
      [&](ite, term guard, term iftrue, term iffalse) -> result<term> {
        auto guardv = cast<boolean>(evaluate(guard));
        if(!guardv)
          return guardv;

        auto truev = evaluate(iftrue);
        if(!truev)
          return truev;

        auto falsev = evaluate(iffalse);
        if(!falsev)
          return falsev;
        
        if(guardv->value())
          return truev;
        return falsev;
      },
      [&](minus, term arg) -> result<term> {
        auto v = evaluate(arg);
        if(!v)
          return v;
        
        return match(*v)(
          [&](integer, auto value) {
            return sigma->integer(-value);
          },
          [&](real, auto value) {
            return sigma->real(-value);
          }
        );
      },
      [&](arithmetic auto op, term left, term right) -> result<term> {
        auto lv = evaluate(left);
        if(!lv)
          return lv;
        auto rv = evaluate(right);
        if(!rv)
          return rv;

        return match(lv)(
          [&](integer) {
            int64_t lhs = cast<integer>(lv)->value();
            int64_t rhs = cast<integer>(rv)->value();
            return sigma->integer(
              match(op)(
                [&](sum) { return lhs + rhs; },
                [&](product) { return lhs * rhs; },
                [&](difference) { return lhs - rhs; },
                [&](division) { return lhs / rhs; }
              )
            );
          },
          [&](real) {
            double lhs = cast<real>(lv)->value();
            double rhs = cast<real>(rv)->value();
            return sigma->real(
              match(op)(
                [&](sum) { return lhs + rhs; },
                [&](product) { return lhs * rhs; },
                [&](difference) { return lhs - rhs; },
                [&](division) { return lhs / rhs; }
              )
            );
          }
        );
      },
      [&](relational auto op, term left, term right) -> result<term> {
        auto lv = evaluate(left);
        if(!lv)
          return lv;
        auto rv = evaluate(right);
        if(!rv)
          return rv;

        return match(lv)(
          [&](integer) {
            int64_t lhs = cast<integer>(lv)->value();
            int64_t rhs = cast<integer>(rv)->value();
            return sigma->boolean(
              match(op)(
                [&](less_than) { return lhs < rhs; },
                [&](less_than_eq) { return lhs <= rhs; },
                [&](greater_than) { return lhs > rhs; },
                [&](greater_than_eq) { return lhs >= rhs; }
              )
            );
          },
          [&](real) {
            double lhs = cast<real>(lv)->value();
            double rhs = cast<real>(rv)->value();
            return sigma->boolean(
              match(op)(
                [&](less_than) { return lhs < rhs; },
                [&](less_than_eq) { return lhs <= rhs; },
                [&](greater_than) { return lhs > rhs; },
                [&](greater_than_eq) { return lhs >= rhs; }
              )
            );
          }
        );
      }
    );
  }

  scope::result<bool> scope::is_type(term t) const {
    result<term> type = type_of(t);
    if(!type)
      return type.error();
    
    return cast<type_type>(type).has_value();
  }

}

