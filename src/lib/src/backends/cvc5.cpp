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
#include <black/backends/cvc5>

#include <cvc5/cvc5.h>

#include <algorithm>
#include <ranges>
#include <iostream>

namespace black::backends::cvc5 {

  using namespace black::support;
  using namespace black::logic;

  using label = black::ast::core::label;

  namespace CVC5 = ::cvc5;

  struct solver::impl_t : processing::consumer {

    struct frame_t {
      persistent::map<entity const *, CVC5::Term> objects;
      persistent::map<CVC5::Term, entity const *> consts;

      bool operator==(frame_t const&) const = default;
    };
  
    module mod;
    std::deque<frame_t> stack = { frame_t{} };
    std::unique_ptr<CVC5::Solver> slv = std::make_unique<CVC5::Solver>();
    bool ignore_push = false;

    impl_t() { 
      slv->setOption("fmf-fun", "true");
      slv->setOption("produce-models", "true");
      slv->push();
    }

    std::optional<CVC5::Term> get_const(entity const *e) const {
      for(auto it = stack.rbegin(); it != stack.rend(); it++)
        if(auto p = it->objects.find(e); p)
          return *p;
      return {};
    }
    
    entity const *get_entity(CVC5::Term t) const {
      for(auto it = stack.rbegin(); it != stack.rend(); it++)
        if(auto p = it->consts.find(t); p)
          return *p;
      return nullptr;
    }

    CVC5::Sort to_sort(term type) const {
      return match(type)(
        [&](logic::error, term, auto err) {
          std::cerr << "error: " << err << "\n";
          return CVC5::Sort();
        },
        [&](integer_type) { return slv->getIntegerSort(); },
        [&](real_type) { return slv->getRealSort(); },
        [&](boolean_type) { return slv->getBooleanSort(); },
        [&](function_type, auto args, term range) {
          CVC5::Sort range_s = to_sort(range);
          std::vector<CVC5::Sort> args_s;
          for(auto arg : args)
            args_s.push_back(to_sort(arg));

          return slv->mkFunctionSort(args_s, range_s);
        }
      );
    }

    std::vector<CVC5::Term> 
    to_term(
      std::vector<term> const& ts, 
      immer::map<variable, CVC5::Term> const& vars
    ) const 
    {
      std::vector<CVC5::Term> result;
      for(term t : ts)
        result.push_back(to_term(t, vars));
      return result;
    }

    CVC5::Term 
    to_term(term t, immer::map<variable, CVC5::Term> const& vars) const {
      return match(t)(
        [&](integer, int64_t v) { return slv->mkInteger(v); },
        [&](real, double v) { 
          auto [num, den] = support::double_to_fraction(v);
          return slv->mkReal(num, den);
        },
        [&](boolean, bool v) { return slv->mkBoolean(v); },
        [&](variable x) {
          CVC5::Term const *var = vars.find(x);
          
          black_assert(var != nullptr);
          return *var;
        },
        [&](object o) {
          std::optional<CVC5::Term> obj = get_const(o.entity());

          black_assert(obj.has_value()); // TODO: handle error well

          return *obj;
        },
        [&](equal, auto args) {
          if(args.size() < 2)
            return slv->mkTrue();

          return slv->mkTerm(CVC5::Kind::EQUAL, to_term(args, vars));
        },
        [&](distinct, auto args) {
          if(args.size() < 2)
            return slv->mkFalse();

          return slv->mkTerm(CVC5::Kind::DISTINCT, to_term(args, vars));
        },
        [&](atom, term head, auto args) {
          std::vector<CVC5::Term> argterms = { to_term(head, vars) };
          for(auto arg : args)
            argterms.push_back(to_term(arg, vars));
          
          return slv->mkTerm(CVC5::Kind::APPLY_UF, argterms);
        },
        [&]<any_of<exists,forall,lambda> T>(T v, auto decls, term body) {
          std::vector<CVC5::Term> varlist;
          immer::map<variable, CVC5::Term> newvars = vars;

          for(auto [name, type] : decls) {
            CVC5::Term term = slv->mkVar(to_sort(type));
            varlist.push_back(term);
            newvars = newvars.set(name, term);
          }

          CVC5::Kind kind = match(v)(
            [](forall) { return CVC5::Kind::FORALL; },
            [](exists) { return CVC5::Kind::EXISTS; },
            [](lambda) { return CVC5::Kind::LAMBDA; }
          );

          return slv->mkTerm(kind, { 
            slv->mkTerm(CVC5::Kind::VARIABLE_LIST, varlist), 
            to_term(body, newvars)
          });
        },
        [&](negation, term arg) {
          return slv->mkTerm(CVC5::Kind::NOT, { to_term(arg, vars) });
        },
        [&](conjunction, auto const& args) {
          return slv->mkTerm(CVC5::Kind::AND, to_term(args, vars));
        },
        [&](disjunction, auto const& args) {
          return slv->mkTerm(CVC5::Kind::OR, to_term(args, vars));
        },
        [&](implication, term left, term right) {
          return slv->mkTerm(
            CVC5::Kind::IMPLIES, { to_term(left, vars), to_term(right, vars) }
          );
        },
        [&](ite, term guard, term iftrue, term iffalse) {
          return slv->mkTerm(CVC5::Kind::ITE, {
            to_term(guard, vars), to_term(iftrue, vars), to_term(iffalse, vars)
          });
        },
        [&](arithmetic auto v, auto ...args) {
          CVC5::Kind kind = match(v)(
            [](minus) { return CVC5::Kind::NEG; },
            [](sum) { return CVC5::Kind::ADD; },
            [](product) { return CVC5::Kind::MULT; },
            [](difference) { return CVC5::Kind::SUB; },
            [&](division) {
              if((cast<integer_type>(type_of(args)) || ...))
                return CVC5::Kind::INTS_DIVISION;
              return CVC5::Kind::DIVISION; 
            }
          );
          return slv->mkTerm(kind, { to_term(args, vars)... });
        },
        [&](relational auto v, term left, term right) {
          CVC5::Kind kind = match(v)(
            [](less_than) { return CVC5::Kind::LT; },
            [](less_than_eq) { return CVC5::Kind::LEQ; },
            [](greater_than) { return CVC5::Kind::GT; },
            [](greater_than_eq) { return CVC5::Kind::GEQ; }
          );
          return 
            slv->mkTerm(kind, { to_term(left, vars), to_term(right, vars) });
        }
      );
    }

    template<typename Node, typename ...Args>
    term from_many_children(CVC5::Term t, Args ...args) {
      std::vector<term> children;
      size_t i = 0;
      for(auto child : t)
        if(i++ >= sizeof...(Args))
          children.push_back(from_term(child));
      return Node(args..., children);
    }

    template<typename Node, typename ...Args>
    term from_children(CVC5::Term t, Args ...args) {
      return [&]<size_t ...Idx>(std::index_sequence<Idx...>) {
        return Node(args..., from_term(t[sizeof...(Args) + Idx])...);
      }(std::make_index_sequence<std::tuple_size_v<Node> - sizeof...(Args)>{});
    }

    term from_term(CVC5::Term t) {
      if(auto p = get_entity(t); p)
        return object(p);

      if(t.isUInt64Value())
        return integer(t.getUInt64Value());
      if(t.isInt64Value())
        return integer(int64_t(t.getUInt64Value()));  
      if(t.isBooleanValue()) 
        return boolean(t.getBooleanValue());
      if(t.isReal32Value()) {
        auto [num, den] = t.getReal32Value();
        return real((double) num / den);
      }
      
      switch(t.getKind()) {
        case CVC5::Kind::EQUAL:
          return from_many_children<equal>(t);
        case CVC5::Kind::DISTINCT:
          return from_many_children<distinct>(t);
        case CVC5::Kind::APPLY_UF:
          return from_many_children<atom>(t, from_term(t[0]));
        case CVC5::Kind::NOT:
          return from_children<negation>(t);
        case CVC5::Kind::AND:
          return from_many_children<conjunction>(t);
        case CVC5::Kind::OR:
          return from_many_children<disjunction>(t);
        case CVC5::Kind::IMPLIES:
          return from_children<implication>(t);
        case CVC5::Kind::ITE:
          return from_children<ite>(t);
        case CVC5::Kind::NEG:
          return from_children<minus>(t);
        case CVC5::Kind::ADD:
          return from_children<sum>(t);
        case CVC5::Kind::MULT:
          return from_children<product>(t);
        case CVC5::Kind::SUB:
          return from_children<difference>(t);
        case CVC5::Kind::INTS_DIVISION:
        case CVC5::Kind::DIVISION:
          return from_children<division>(t);
        case CVC5::Kind::LT:
          return from_children<less_than>(t);
        case CVC5::Kind::LEQ:
          return from_children<less_than_eq>(t);
        case CVC5::Kind::GT:
          return from_children<greater_than>(t);
        case CVC5::Kind::GEQ:
          return from_children<greater_than_eq>(t);
        default:
          black_unreachable();
      }
    }

    virtual void import(module m) override {
      auto _ = support::checkpoint(ignore_push);
      ignore_push = true;

      module empty;
      m.replay(empty, this);
    } 

    void define(entity const *e) {
      black_assert(e->value);

      CVC5::Term t = match(*e->value)(
        [&](lambda, auto const &decls, term body) {
          std::vector<CVC5::Term> vars;
          immer::map<variable, CVC5::Term> varmap;

          for(auto [name, type] : decls) {
            CVC5::Term var = slv->mkVar(to_sort(type));
            vars.push_back(var);
            varmap = varmap.set(name, var);
          }

          auto fun_ty = cast<function_type>(e->type);
          black_assert(fun_ty.has_value());

          return slv->defineFun(
            std::format("{}", e->name.name()), vars, 
            to_sort(fun_ty->range()), to_term(body, varmap)
          );
        },
        [&](auto d) {
          return slv->defineFun(
            std::format("{}", e->name.name()), 
            {}, to_sort(e->type), to_term(d, {})
          );
        }
      );

      stack.back().objects.insert({e, t});
      stack.back().consts.insert({t, e});
    }

    void define(std::vector<entity const *> lookups) {
      std::vector<CVC5::Term> names;
      for(auto e : lookups) {
        CVC5::Term name = slv->mkConst(to_sort(e->type));
        names.push_back(name);
        stack.back().objects.insert({e, name});
        stack.back().consts.insert({name, e});
      }

      std::vector<std::vector<CVC5::Term>> vars;
      std::vector<CVC5::Term> bodies;
      for(auto e : lookups) {
        std::vector<CVC5::Term> thesevars;
        
        if(cast<function_type>(e->type)) {
          function_type ty = unwrap(cast<function_type>(e->type));
          lambda fun = unwrap(cast<lambda>(e->value));

          immer::map<variable, CVC5::Term> boundvars;
          for(decl d : fun.vars()) {
            CVC5::Term var = slv->mkVar(to_sort(d.type));
            boundvars = boundvars.insert({d.name, var});
            thesevars.push_back(var);
          }
          vars.push_back(thesevars);

          bodies.push_back(to_term(fun.body(), boundvars));
        } else {
          thesevars.push_back({});
          bodies.push_back(to_term(unwrap(e->value), {}));
        }
      }

      return slv->defineFunsRec(names, vars, bodies);
    }

    void declare(entity const *e) {
      black_assert(!e->value);
      
      CVC5::Term t = slv->mkConst(to_sort(e->type));
      
      stack.back().objects.insert({e, t});
      stack.back().consts.insert({t, e});
    }

    virtual void adopt(std::shared_ptr<root const> r) override {
      // first, declare the declarations and collect the definitions
      std::vector<entity const *> defs;
      for(auto const& e : r->entities) {
        if(!e->value.has_value())
          declare(e.get());
        else
          defs.push_back(e.get());
      }

      // then, define the definitions (recursively or not...)
      if(r->mode == recursion::forbidden)
        for(auto d : defs)
          define(d);
      else
        define(defs);
    }

    virtual void require(term t) override {
      slv->assertFormula(to_term(t, {}));
    }

    virtual void push() override {
      if(ignore_push)
        return;

      if(stack.empty())
        stack.push_back({});
      else
        stack.push_back(stack.back());
      slv->push();
    }

    virtual void pop(size_t n) override {
      if(ignore_push)
        return;

      for(size_t i = 0; i < n; i++)
        stack.pop_back();
      slv->pop(unsigned(n));
    }

    support::tribool check(module m) {
      m.replay(mod, this);
      mod = std::move(m);

      CVC5::Result res = slv->checkSat();
      if(res.isSat())
        return true;
      if(res.isUnsat())
        return false;
        
      return tribool::undef;
    }

    std::optional<term> value(term t) 
    {
      CVC5::Term v = slv->getValue(to_term(t, {}));
      if(v.isNull())
        return {};
      
      return from_term(v);
    }

  };


  solver::solver() : _impl{std::make_unique<impl_t>()} { }

  solver::~solver() = default;

  support::tribool solver::check(module m) { 
    return _impl->check(std::move(m)); 
  }

  std::optional<term> solver::value(term t) {
    return _impl->value(t);
  }

}