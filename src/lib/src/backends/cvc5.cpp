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

#include <cvc5/cvc5.h>

namespace black::backends::cvc5 {

  using namespace black::support;
  using namespace black::logic;

  using label = black::ast::core::label;

  namespace CVC5 = ::cvc5;

  struct solver::impl_t {
    std::vector<module> imports;

    CVC5::Solver slv;

    struct frame_t {
      immer::map<object, CVC5::Term> objects;
      immer::map<CVC5::Term, object> consts;
      immer::vector<term> requirements;
    } frame;
  
    std::stack<frame_t> stack;    

    CVC5::Sort to_sort(term type) const {
      return match(type)(
        [&](integer_type) { return slv.getIntegerSort(); },
        [&](real_type) { return slv.getRealSort(); },
        [&](boolean_type) { return slv.getBooleanSort(); },
        [&](function_type, auto args, term range) {
          CVC5::Sort range_s = to_sort(range);
          std::vector<CVC5::Sort> args_s;
          for(auto arg : args)
            args_s.push_back(to_sort(arg));

          return slv.mkFunctionSort(args_s, range_s);
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
        [&](integer, int64_t v) { return slv.mkInteger(v); },
        [&](real, double v) { 
          auto [num, den] = support::double_to_fraction(v);
          return slv.mkReal(num, den);
        },
        [&](boolean, bool v) { return slv.mkBoolean(v); },
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
            return slv.mkTrue();

          return slv.mkTerm(CVC5::Kind::EQUAL, to_term(args, vars));
        },
        [&](distinct, auto args) {
          if(args.size() < 2)
            return slv.mkFalse();

          return slv.mkTerm(CVC5::Kind::DISTINCT, to_term(args, vars));
        },
        [&](atom, term head, auto args) {
          std::vector<CVC5::Term> argterms = { to_term(head, vars) };
          for(auto arg : args)
            argterms.push_back(to_term(arg, vars));
          
          return slv.mkTerm(CVC5::Kind::APPLY_UF, argterms);
        },
        [&](quantifier auto q, auto decls, term body) {
          std::vector<CVC5::Term> varlist;
          immer::map<label, CVC5::Term> newvars = vars;

          for(auto [name, type] : decls) {
            CVC5::Term term = slv.mkVar(to_sort(type));
            varlist.push_back(term);
            newvars = newvars.set(name, term);
          }

          CVC5::Kind kind = match(q)(
            [](forall) { return CVC5::Kind::FORALL; },
            [](exists) { return CVC5::Kind::EXISTS; }
          );

          return slv.mkTerm(kind, { 
            slv.mkTerm(CVC5::Kind::VARIABLE_LIST, varlist), 
            to_term(body, newvars)
          });
        },
        [&](negation, term arg) {
          return slv.mkTerm(CVC5::Kind::NOT, { to_term(arg, vars) });
        }
      );
    }

    void collect(module const& m) {
      for(module imported : m.imports())
        collect(imported);
      
      for(object obj : m.objects()) {
        CVC5::Term t = slv.mkConst(to_sort(obj.lookup()->type));
        frame.objects = frame.objects.insert({obj, t});
        frame.consts = frame.consts.insert({t, obj});
      }
    }

    void import(module m) {
      imports.push_back(m);
      collect(m);
    }

    void require(term r) {
      frame.requirements = frame.requirements.push_back(r);
    }

    void push() {
      stack.push(frame);
      slv.push();
    }

    void pop() {
      if(stack.empty()) {
        frame = {};
        slv.resetAssertions();
      }

      frame = stack.top();
      stack.pop();
      slv.pop();
    }

    support::tribool check() {
      return true;
    }
    
    support::tribool check_with(term) {
      return true;
    }

  };


  solver::solver() 
    : _impl{std::make_unique<impl_t>()} { }

  solver::~solver() = default;

  void solver::import(module m) { _impl->import(std::move(m)); }

  void solver::require(term r) { _impl->require(r); }

  void solver::push() { _impl->push(); }

  void solver::pop() { _impl->pop(); }

  support::tribool solver::check() { return _impl->check(); }
    
  support::tribool solver::check_with(term t) {
    return _impl->check_with(t);
  }

}