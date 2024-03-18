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
#include <black/backends/cvc5>

#include <immer/vector.hpp>
#include <immer/map.hpp>
#include <immer/set.hpp>

#include <cvc5/cvc5.h>

#include <algorithm>

namespace black::backends::cvc5 {

  using namespace black::support;
  using namespace black::logic;

  using label = black::ast::core::label;

  namespace CVC5 = ::cvc5;

  struct solver::impl_t {

    struct frame_t {
      immer::map<object, CVC5::Term> objects;
      immer::map<CVC5::Term, object> consts;
      immer::set<term> reqs;
    } frame;
  
    alphabet *sigma;
    module mod;
    std::unique_ptr<CVC5::Solver> slv;
    std::stack<frame_t> stack;    

    impl_t(alphabet *sigma) 
      : sigma{sigma}, mod{sigma}, slv{std::make_unique<CVC5::Solver>()} { }

    CVC5::Sort to_sort(term type) const {
      return match(type)(
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
      immer::map<label, CVC5::Term> const& vars
    ) const 
    {
      std::vector<CVC5::Term> result;
      for(term t : ts)
        result.push_back(to_term(t, vars));
      return result;
    }

    CVC5::Term 
    to_term(term t, immer::map<label, CVC5::Term> const& vars) const {
      return match(t)(
        [&](integer, int64_t v) { return slv->mkInteger(v); },
        [&](real, double v) { 
          auto [num, den] = support::double_to_fraction(v);
          return slv->mkReal(num, den);
        },
        [&](boolean, bool v) { return slv->mkBoolean(v); },
        [&](variable x) {
          CVC5::Term const *var = vars.find(x.name());
          
          black_assert(var != nullptr);
          return *var;
        },
        [&](object o) {
          CVC5::Term const *obj = frame.objects.find(o);

          black_assert(obj != nullptr); // TODO: handle error well

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
          immer::map<label, CVC5::Term> newvars = vars;

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

    void add(object obj) {
      if(frame.objects.find(obj) != nullptr)
        return;

      auto lu = obj.lookup();
      CVC5::Term t = !lu->value ? slv->mkConst(to_sort(lu->type)) : 
        match(*lu->value)(
          [&](lambda, auto const &decls, term body) {
            std::vector<CVC5::Term> vars;
            immer::map<label, CVC5::Term> varmap;

            for(auto [name, type] : decls) {
              CVC5::Term var = slv->mkVar(to_sort(type));
              vars.push_back(var);
              varmap = varmap.set(name, var);
            }

            auto fun_ty = cast<function_type>(lu->type);
            black_assert(fun_ty.has_value());

            return slv->defineFun(
              std::format("{}", lu->name), vars, 
              to_sort(fun_ty->range()), to_term(body, varmap)
            );
          },
          [&](auto d) {
            return slv->defineFun(
              std::format("{}", lu->name), {}, to_sort(lu->type), to_term(d, {})
            );
          }
        ); 
      
      frame.objects = frame.objects.insert({obj, t});
      frame.consts = frame.consts.insert({t, obj});
    }

    void add(term t) {
      if(frame.reqs.find(t) != nullptr)
        return;

      frame.reqs = frame.reqs.insert(t);
      slv->assertFormula(to_term(t, {}));
    }

    void add(module const& m) {
      for(object obj : m.objects())
        add(obj);

      for(term req : m.requirements())
        add(req);
    }

    void traverse(module const& m) {
      for(module imported : m.imports())
        add(imported);

      add(m);
    }

    void push() {
      stack.push(frame);
      slv->push();
    }

    void clear() {
      *this = impl_t{sigma};
    }

    void pop() {
      if(stack.empty()) {
        clear();
      } else {
        frame = stack.top();
        stack.pop();
        slv->pop();
      }
    }

    void rollback(module m) {
      while(!m.contains(mod)) {
        pop();
        mod.pop();
      }

      std::vector<module> steps;
      while(!mod.contains(m)) {
        steps.push_back(m);
        m.pop();
      }

      for(auto it = rbegin(steps); it != rend(steps); it++) {
        push();
        add(*it);
      }
    }

    support::tribool check(module m) {
      if(m.imports() != mod.imports()) { // TODO: use structural sharing
        clear();
        traverse(m);
      } else {
        rollback(m);
      }

      mod = m;

      CVC5::Result res = slv->checkSat();
      if(res.isSat())
        return true;
      if(res.isUnsat())
        return false;
      return tribool::undef;
    }

  };


  solver::solver(alphabet *sigma) 
    : _impl{std::make_unique<impl_t>(sigma)} { }

  solver::~solver() = default;

  support::tribool solver::check(module m) { 
    return _impl->check(std::move(m)); 
  }

}