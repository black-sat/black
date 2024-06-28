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
#include <black/solvers/cvc5>

#include <cvc5/cvc5.h>

#include <algorithm>
#include <ranges>
#include <cmath>

namespace black::solvers {

  using namespace black::support;
  using namespace black::logic;

  using label = black::ast::core::label;

  namespace CVC5 = ::cvc5;

  struct cvc5_t::impl_t : pipes::consumer 
  {
    persistent::map<entity const *, CVC5::Term> objects;
    persistent::map<CVC5::Term, entity const *> consts;
    std::unique_ptr<CVC5::Solver> slv = std::make_unique<CVC5::Solver>();
    bool ignore_push = false;

    impl_t() { 
      slv->setOption("fmf-fun", "true");
      slv->setOption("produce-models", "true");
    }

    void set_smt_logic(std::string const& logic) {
      slv->setLogic(logic);
    }

    std::optional<CVC5::Term> get_const(entity const *e) const {
      if(auto p = objects.find(e); p)
        return *p;
      return {};
    }
    
    entity const *get_entity(CVC5::Term t) const {
      if(auto p = consts.find(t); p)
        return *p;  
      return nullptr;
    }

    CVC5::Sort to_sort(type ty) const {
      return ast::traverse<CVC5::Sort>(ty)(
        [&](types::error) {
          return CVC5::Sort();
        },
        [&](types::integer) { return slv->getIntegerSort(); },
        [&](types::real) { return slv->getRealSort(); },
        [&](types::boolean) { return slv->getBooleanSort(); },
        [&](types::function, auto args, auto range) {
          return slv->mkFunctionSort(args, range);
        }
      );
    }

    CVC5::Term 
    to_term(term t, persistent::map<variable, CVC5::Term> vars) const 
    {
      return ast::traverse<CVC5::Term>(t)(
        [&](integer, int64_t v) { return slv->mkInteger(v); },
        [&](real, double v) { 
          double abs = std::abs(v);
          auto [num, den] = support::double_to_fraction(abs);
          if(v >= 0)
            return slv->mkReal(num, den);
          return slv->mkReal(-num, den);
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

          return slv->mkTerm(CVC5::Kind::EQUAL, std::move(args));
        },
        [&](distinct, auto args) {
          if(args.size() < 2)
            return slv->mkFalse();

          return slv->mkTerm(CVC5::Kind::DISTINCT, std::move(args));
        },
        [&](atom, auto head, auto args) {
          args.insert(args.begin(), head);
          return slv->mkTerm(CVC5::Kind::APPLY_UF, std::move(args));
        },
        [&](any_of<exists,forall,lambda> auto v, auto decls/*, auto body*/) {
          std::vector<CVC5::Term> varlist;
          
          persistent::map<variable, CVC5::Term> newvars = vars;

          for(auto [name, type] : decls) {
            CVC5::Term term = slv->mkVar(to_sort(type));
            varlist.push_back(term);
            newvars.set(name, term);
          }

          CVC5::Kind kind = match(v)(
            [](forall) { return CVC5::Kind::FORALL; },
            [](exists) { return CVC5::Kind::EXISTS; },
            [](lambda) { return CVC5::Kind::LAMBDA; }
          );

          return slv->mkTerm(kind, { 
            slv->mkTerm(CVC5::Kind::VARIABLE_LIST, varlist), 
            to_term(v.body(), newvars)
          });
        },
        [&](negation, auto arg) {
          return slv->mkTerm(CVC5::Kind::NOT, { arg });
        },
        [&](conjunction, auto args) {
          return slv->mkTerm(CVC5::Kind::AND, std::move(args));
        },
        [&](disjunction, auto args) {
          return slv->mkTerm(CVC5::Kind::OR, std::move(args));
        },
        [&](implication, auto left, auto right) {
          return slv->mkTerm(CVC5::Kind::IMPLIES, { left, right });
        },
        [&](ite, auto guard, auto iftrue, auto iffalse) {
          return slv->mkTerm(CVC5::Kind::ITE, { guard, iftrue, iffalse });
        },
        [&](arithmetic auto v, auto ...args) {
          CVC5::Kind kind = match(v)(
            [](minus) { return CVC5::Kind::NEG; },
            [](sum) { return CVC5::Kind::ADD; },
            [](product) { return CVC5::Kind::MULT; },
            [](difference) { return CVC5::Kind::SUB; },
            [&](division) {
              if((args.getSort().isInteger() || ...))
                return CVC5::Kind::INTS_DIVISION;
              return CVC5::Kind::DIVISION; 
            }
          );
          return slv->mkTerm(kind, { args... });
        },
        [&](relational auto v, auto left, auto right) {
          CVC5::Kind kind = match(v)(
            [](less_than) { return CVC5::Kind::LT; },
            [](less_than_eq) { return CVC5::Kind::LEQ; },
            [](greater_than) { return CVC5::Kind::GT; },
            [](greater_than_eq) { return CVC5::Kind::GEQ; }
          );
          return slv->mkTerm(kind, { left, right });
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

      if(t.getSort().isInteger() && t.isUInt64Value())
        return integer(t.getUInt64Value());
      
      if(t.getSort().isInteger() && t.isInt64Value())
        return integer(int64_t(t.getUInt64Value()));  
      
      if(t.getSort().isBoolean() && t.isBooleanValue()) 
        return boolean(t.getBooleanValue());

      if(t.getSort().isReal() && t.isReal32Value()) {
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
          persistent::map<variable, CVC5::Term> varmap;

          for(auto [name, type] : decls) {
            CVC5::Term var = slv->mkVar(to_sort(type));
            vars.push_back(var);
            varmap.set(name, var);
          }

          auto fun_ty = cast<types::function>(e->type);
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

      objects.insert({e, t});
      consts.insert({t, e});
    }

    void define(std::vector<entity const *> lookups) {
      std::vector<CVC5::Term> names;
      for(auto e : lookups) {
        CVC5::Term name = slv->mkConst(to_sort(e->type));
        names.push_back(name);
        objects.insert({e, name});
        consts.insert({name, e});
      }

      std::vector<std::vector<CVC5::Term>> vars;
      std::vector<CVC5::Term> bodies;
      for(auto e : lookups) {
        std::vector<CVC5::Term> thesevars;
        
        if(cast<types::function>(e->type)) {
          types::function ty = unwrap(cast<types::function>(e->type));
          lambda fun = unwrap(cast<lambda>(e->value));

          persistent::map<variable, CVC5::Term> boundvars;
          for(decl d : fun.vars()) {
            CVC5::Term var = slv->mkVar(to_sort(d.type));
            boundvars.insert({d.name, var});
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
      
      objects.insert({e, t});
      consts.insert({t, e});
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

    virtual void state(term t, statement s) override {
      black_assert(s == statement::requirement);
      slv->assertFormula(to_term(t, {}));
    }

    virtual void push() override {
      if(ignore_push)
        return;

      objects.push();
      consts.push();
      slv->push();
    }

    virtual void pop(size_t n) override {
      if(ignore_push)
        return;

      objects.pop(n);
      consts.pop(n);
      slv->pop(unsigned(n));
    }

    support::tribool check() {
      CVC5::Result res = slv->checkSat();
      if(res.isSat())
        return true;
      if(res.isUnsat())
        return false;
        
      return tribool::undef;
    }

    std::optional<term> value(object x) 
    {
      CVC5::Term v = slv->getValue(to_term(x, {}));
      if(v.isNull())
        return {};
      
      return from_term(v);
    }

  };


  cvc5_t::cvc5_t() : _impl{std::make_unique<impl_t>()} { }

  cvc5_t::~cvc5_t() = default;

  void cvc5_t::set_smt_logic(std::string const&logic) {
    _impl->set_smt_logic(logic);
  }

  pipes::consumer *cvc5_t::consumer() { 
    return _impl.get();
  }

  support::tribool cvc5_t::check() { 
    return _impl->check(); 
  }

  std::optional<term> cvc5_t::value(object x) {
    return _impl->value(x);
  }

}