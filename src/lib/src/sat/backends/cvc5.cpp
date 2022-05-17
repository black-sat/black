//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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


#include <black/sat/backends/cvc5.hpp>

#include <black/sat/backends/fractionals.hpp>

#include <cvc5/cvc5.h>
#include <tsl/hopscotch_map.h>

BLACK_REGISTER_SAT_BACKEND(cvc5, {
  black::sat::feature::smt, black::sat::feature::quantifiers
})

namespace black::sat::backends
{
  inline proposition fresh(formula f) {
    return f.sigma()->prop(f);
  }

  namespace cvc = ::cvc5;
  struct cvc5::_cvc5_t {
    cvc::Solver solver;

    tsl::hopscotch_map<formula, cvc::Term> formulas; 
    tsl::hopscotch_map<term, cvc::Term> terms;

    cvc::Term to_cvc5(
      formula, tsl::hopscotch_map<variable, cvc::Term> const&env
    );
    cvc::Term to_cvc5(
      term, tsl::hopscotch_map<variable, cvc::Term> const&env
    );
    cvc::Sort to_cvc5(sort);

    cvc::Term to_cvc5_inner(formula, 
      tsl::hopscotch_map<variable, cvc::Term> const&env);
    cvc::Term to_cvc5_inner(term,
      tsl::hopscotch_map<variable, cvc::Term> const&env);

    cvc::Term to_cvc5_func_decl(
      alphabet *sigma, std::string const&name, unsigned arity, bool is_relation
    );
  };

  cvc5::cvc5() : _data{std::make_unique<_cvc5_t>()}
  {
    _data->solver.setLogic("ALL");
    _data->solver.setOption("produce-models", "true");
  }

  cvc5::~cvc5() = default;

  void cvc5::assert_formula(formula f) {
    cvc::Term term = _data->to_cvc5(f, {});
    _data->solver.assertFormula(term);
  }

  tribool cvc5::is_sat_with(formula f) 
  {
    cvc::Term term = _data->to_cvc5(f, {});

    cvc::Result res = _data->solver.checkSatAssuming(term);

    return res.isSat() ? tribool{true} :
           res.isUnsat() ? tribool{false} :
           tribool::undef;
  }

  tribool cvc5::is_sat() 
  {
    cvc::Result res = _data->solver.checkSat();

    return res.isSat() ? tribool{true} :
           res.isUnsat() ? tribool{false} :
           tribool::undef;
  }

  tribool cvc5::value(proposition p) const 
  {
    auto it = _data->formulas.find(p);
    if(it == _data->formulas.end())
      return tribool::undef;
    
    cvc::Term term = it->second;
    cvc::Term res = _data->solver.getValue(term);

    if(res.isNull())
      return tribool::undef;

    return res.getBooleanValue();
  }

  void cvc5::clear() {
    _data->solver.resetAssertions();
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5(
    formula f, tsl::hopscotch_map<variable, cvc::Term> const&env
  ) {
    if(auto it = formulas.find(f); it != formulas.end()) 
      return it->second;

    cvc::Term cvc5_t = to_cvc5_inner(f, env);
    formulas.insert({f, cvc5_t});

    return cvc5_t;
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5(
    term t, tsl::hopscotch_map<variable, cvc::Term> const&env
  ) {
    if(auto it = terms.find(t); it != terms.end()) 
      return it->second;

    cvc::Term cvc5_t = to_cvc5_inner(t, env);
    if(!t.is<variable>() || env.find(*t.to<variable>()) == env.end())
      terms.insert({t, cvc5_t});

    return cvc5_t;
  }

  cvc::Sort cvc5::_cvc5_t::to_cvc5(sort s) {
    switch(s){ 
      case sort::Int:
        return solver.getIntegerSort();
      case sort::Real:
        return solver.getRealSort();
    }
    black_unreachable(); // LCOV_EXCL_LINE
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5_inner(
    formula f, tsl::hopscotch_map<variable, cvc::Term> const&env
  ) {
    return f.match(
      [&](boolean b) {
        return b.value() ? solver.mkTrue() : solver.mkFalse();
      },
      [&](atom a) -> cvc::Term {
        std::vector<cvc::Term> cvc_terms;
        for(term t : a.terms())
          cvc_terms.push_back(to_cvc5(t, env));

        // we know how to encode known relations
        if(auto k = a.rel().known_type(); k) {
          black_assert(cvc_terms.size() == 2);
          switch(*k) {
            case relation::equal:
              return solver.mkTerm(cvc::EQUAL, cvc_terms);
            case relation::not_equal:
              return solver.mkTerm(cvc::NOT,
                {solver.mkTerm(cvc::EQUAL, cvc_terms)}
              );
            case relation::less_than:
              return solver.mkTerm(cvc::LT, cvc_terms);
            case relation::less_than_equal:
              return solver.mkTerm(cvc::LEQ, cvc_terms);
            case relation::greater_than:
              return solver.mkTerm(cvc::GT, cvc_terms);
            case relation::greater_than_equal:
              return solver.mkTerm(cvc::GEQ, cvc_terms);
          }
        }

        // Otherwise we go for uninterpreted relations
        cvc::Term rel = 
          to_cvc5_func_decl(
            a.sigma(), a.rel().name(), unsigned(cvc_terms.size()), true);
        
        cvc_terms.insert(cvc_terms.begin(), rel);
        return solver.mkTerm(cvc::APPLY_UF, cvc_terms);
      },
      [&](quantifier q) {
        black_assert(q.sigma()->domain());
        cvc::Term var = 
          solver.mkVar(
              to_cvc5(*q.sigma()->domain()), to_string(q.var().unique_id()));
        
        cvc::Term varlist = solver.mkTerm(cvc::VARIABLE_LIST, {var});

        tsl::hopscotch_map<variable, cvc::Term> new_env = env;
        new_env.insert({q.var(), var});

        if(q.quantifier_type() == quantifier::type::forall)
          return 
            solver.mkTerm(cvc::FORALL, {varlist, to_cvc5(q.matrix(), new_env)});
        else
          return 
            solver.mkTerm(cvc::EXISTS, {varlist, to_cvc5(q.matrix(), new_env)});
      },
      [&](proposition p) {
        return 
          solver.mkConst(solver.getBooleanSort(), to_string(p.unique_id()));
      },
      [&](negation, formula n) {
        return solver.mkTerm(cvc::NOT, {to_cvc5(n, env)});
      },
      [&](big_conjunction c) {
        std::vector<cvc::Term> args;
        for(formula op : c.operands())
          args.push_back(to_cvc5(op, env));

        return solver.mkTerm(cvc::AND, args);
      },
      [&](big_disjunction c) {
        std::vector<cvc::Term> args;
        for(formula op : c.operands())
          args.push_back(to_cvc5(op, env));

        return solver.mkTerm(cvc::OR, args);
      },
      [&](implication, formula left, formula right) {
        return 
          solver.mkTerm(cvc::IMPLIES,{to_cvc5(left, env), to_cvc5(right, env)});
      },
      [&](iff, formula left, formula right) {
        return 
          solver.mkTerm(cvc::EQUAL, {to_cvc5(left, env), to_cvc5(right, env)});
      },
      [](temporal) -> cvc::Term { // LCOV_EXCL_LINE
        black_unreachable(); // LCOV_EXCL_LINE
      }
    );
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5_func_decl(
    alphabet *sigma, std::string const&name, unsigned arity, bool is_relation
  ) {
    black_assert(arity > 0);
    black_assert(sigma->domain().has_value());

    std::vector<cvc::Sort> sorts{arity};
    std::fill(sorts.begin(), sorts.end(), to_cvc5(*sigma->domain()));

    cvc::Sort range = is_relation ? solver.getBooleanSort() : sorts[0];

    cvc::Sort funcSort = solver.mkFunctionSort(sorts, range);

    return solver.mkConst(funcSort, name);
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5_inner(
    term t, tsl::hopscotch_map<variable, cvc::Term> const&env
  ) {
    return t.match(
      [&](constant c) {
        if(std::holds_alternative<int64_t>(c.value())) {
          black_assert(c.sigma()->domain().has_value());

          if(c.sigma()->domain() == sort::Int)
            return solver.mkInteger(std::get<int64_t>(c.value()));
          return solver.mkReal(std::get<int64_t>(c.value()));
        } else {
          auto [num,denum] = 
            black::internal::double_to_fraction(std::get<double>(c.value()));
          return solver.mkReal(num, denum);
        }
      },
      [&](variable v) {
        if(auto it = env.find(v); it != env.end())
          return it->second;

        std::optional<sort> s = t.sigma()->domain();
        black_assert(s.has_value());
        return solver.mkConst(to_cvc5(*s), to_string(v.unique_id()));
      },
      [&](application a) {
        black_assert(a.sigma()->domain().has_value());

        std::vector<cvc::Term> cvc_terms;
        for(term t2 : a.arguments())
          cvc_terms.push_back(to_cvc5(t2, env));

        // We know how to encode known functions
        if(auto k = a.func().known_type(); k) {
          switch(*k) {
            case function::negation:
              black_assert(cvc_terms.size() == 1);
              return solver.mkTerm(cvc::NEG, cvc_terms);
            case function::subtraction:
              return solver.mkTerm(cvc::SUB, cvc_terms);
            case function::addition:
              return solver.mkTerm(cvc::ADD, cvc_terms);
            case function::multiplication:
              return solver.mkTerm(cvc::MULT, cvc_terms);
            case function::division:
              black_assert(cvc_terms.size() == 2);
              return solver.mkTerm(cvc::DIVISION, cvc_terms);
          }
          black_unreachable(); // LCOV_EXCL_LINE
        }

        // Otherwise we go for uninterpreted functions
        cvc::Term func = to_cvc5_func_decl(
          a.sigma(), a.func().name(), unsigned(cvc_terms.size()), false);

        cvc_terms.insert(cvc_terms.begin(), func);
        return solver.mkTerm(cvc::APPLY_UF, cvc_terms);
      },
      // We should not have any next(var) term at this point
      [](next) -> cvc::Term { black_unreachable(); }, // LCOV_EXCL_LINE
      [](wnext) -> cvc::Term { black_unreachable(); } // LCOV_EXCL_LINE
    );
  }

  std::optional<std::string> cvc5::license() const {
    return
R"(
cvc5 is copyright (C) 2009-2021 by its authors and contributors (see the file
AUTHORS) and their institutional affiliations.  All rights reserved.

The source code of cvc5 is open and available to students, researchers,
software companies, and everyone else to study, to modify, and to redistribute
original or modified versions; distribution is under the terms of the modified
BSD license (reproduced below).  Please note that cvc5 can be configured
(however, by default it is not) to link against some GPLed libraries, and
therefore the use of these builds may be restricted in non-GPL-compatible
projects.  See below for a discussion of CLN and GLPK (the two GPLed optional
library dependences for cvc5), and how to ensure you have a build that doesn't
link against GPLed libraries.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT OWNERS AND CONTRIBUTORS
''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
)";
  }

}