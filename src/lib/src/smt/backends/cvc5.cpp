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
#include <black/smt>

#include <cvc5/cvc5.h>

namespace black::smt::cvc5 {

  using namespace black::support;
  using namespace black::logic;

  namespace CVC5 = ::cvc5;

  struct solver::impl_t {
    module const *mod;

    CVC5::Solver slv;

    support::map<term, CVC5::Sort> sorts;
    support::map<term, CVC5::Term> terms;
    std::stack<support::map<variable, CVC5::Term>> vars;

    impl_t(module const *m) : mod{m} { }

    CVC5::Sort to_cvc5_sort(term);
    CVC5::Term to_cvc5(term);

    std::vector<CVC5::Term> to_cvc5(std::vector<term> const& vec) {
      std::vector<CVC5::Term> result;
      for(auto t : vec)
        result.push_back(to_cvc5(t));
      return result;
    }

  };

  CVC5::Sort solver::impl_t::to_cvc5_sort(term t) {
    if(auto it = sorts.find(t); it != sorts.end())
      return it->second;

    auto cache = [&](CVC5::Sort v) {
      sorts.insert({t, v});
      return v;
    };

    return cache(
      match(t)(
        [&](integer_type) { return slv.getIntegerSort(); },
        [&](real_type) { return slv.getRealSort(); },
        [&](boolean_type) { return slv.getBooleanSort(); },
        [&](function_type, auto args, term range) {
          CVC5::Sort range_s = to_cvc5_sort(range);
          std::vector<CVC5::Sort> args_s;
          for(auto arg : args)
            args_s.push_back(to_cvc5_sort(arg));

          return slv.mkFunctionSort(args_s, range_s);
        }
      )
    );
  }

  //
  // TODO: the caching of terms in presence of quantifiers is currently wrong
  // because mkVar() in cvc5 always returns a fresh variable while our
  // `variable` terms are uniqued by name.
  //
  // TODO: constants must not be declared on-the-fly. Rather, we need to declare
  // all the constants corresponding to all the module's declarations upfront,
  // to be able to ask any of them to the solver after the solving process.
  //
  // To do that we need to be able to get the imports from a module, so the 
  // current opaque handling of module::impl_t is unsuitable. But:
  // 1. do we want modules to have value semantics?
  // 2. is it ok to just keep raw pointers to modules?
  //
  CVC5::Term solver::impl_t::to_cvc5(term t) {
    if(auto it = terms.find(t); it != terms.end())
      return it->second;

    auto cache = [&](CVC5::Term v) {
      terms.insert({t, v});
      return v;
    };

    return match(t)(
      [&](integer, int64_t v) { return cache(slv.mkInteger(v)); },
      [&](real, double v) { 
        auto [num, den] = support::double_to_fraction(v);
        return cache(slv.mkReal(num, den));
      },
      [&](boolean, bool v) { return slv.mkBoolean(v); },
      [&](variable v) {
        black_assert(!this->vars.empty()); // TODO: raise error
        
        auto it = this->vars.top().find(v);
        black_assert(it != this->vars.top().end()); // TODO: raise error

        return it->second;
      },
      [&](object, auto decl) {
        std::string name = std::format("{}", decl->name);
        return cache(slv.mkConst(to_cvc5_sort(decl->type), name));
      },
      [&](equal, auto args) {
        if(args.size() < 2)
          return cache(slv.mkTrue());

        return cache(slv.mkTerm(CVC5::Kind::EQUAL, to_cvc5(args)));
      },
      [&](distinct, auto args) {
        if(args.size() < 2)
          return cache(slv.mkFalse());

        return cache(slv.mkTerm(CVC5::Kind::DISTINCT, to_cvc5(args)));
      },
      [&](atom, term head, auto args) {
        std::vector<CVC5::Term> argterms = { to_cvc5(head) };
        for(auto arg : args)
          argterms.push_back(to_cvc5(arg));
        
        return cache(slv.mkTerm(CVC5::Kind::APPLY_UF, argterms));
      },
      [&](quantifier auto q, auto binds, term body) {
        support::map<variable, CVC5::Term> varmap;
        std::vector<CVC5::Term> varlist;

        for(auto [var, type] : binds) {
          std::string name = std::format("{}", var.name());
          CVC5::Term term = slv.mkVar(to_cvc5_sort(type), name);
          varmap.insert({var, term});
          varlist.push_back(term);
        }

        CVC5::Kind kind = match(q)(
          [](forall) { return CVC5::Kind::FORALL; },
          [](exists) { return CVC5::Kind::EXISTS; }
        );

        this->vars.push(std::move(varmap));
        
        return slv.mkTerm(kind, { 
          slv.mkTerm(CVC5::Kind::VARIABLE_LIST, varlist), to_cvc5(body)
        });
      }
    );
  }

  solver::solver(module const *mod) 
    : _impl{std::make_unique<impl_t>(mod)} { }

  solver::~solver() = default;

  tribool solver::check() {
    return true;
  }

}