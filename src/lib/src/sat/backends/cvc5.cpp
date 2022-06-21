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
#include <black/logic/prettyprint.hpp>

#include <black/sat/backends/fractionals.hpp>

#include <cvc5/cvc5.h>
#include <tsl/hopscotch_map.h>

BLACK_REGISTER_SAT_BACKEND(cvc5, {
  black::sat::feature::smt, black::sat::feature::quantifiers
})

namespace black_internal::cvc5
{
  inline proposition fresh(formula f) {
    return f.sigma()->proposition(f);
  }

  struct cvc5_decl_key {
    std::string name;
    unsigned arity;
    bool is_relation;

    friend 
    bool operator==(cvc5_decl_key const&, cvc5_decl_key const&) = default;
  };

  } namespace std {
  
    template<>
    struct hash<black_internal::cvc5::cvc5_decl_key> {
      size_t operator()(black_internal::cvc5::cvc5_decl_key const& k) const {
        using namespace black_internal;

        size_t h1 = std::hash<std::string>{}(k.name);
        size_t h2 = std::hash<unsigned>{}(k.arity);
        size_t h3 = std::hash<bool>{}(k.is_relation);

        return hash_combine(h1, hash_combine(h2, h3));
      }
    };

  } namespace black_internal::cvc5 {

  namespace cvc = ::cvc5;
  struct cvc5::_cvc5_t {
    cvc::Solver solver;
    bool sat_response = false;

    tsl::hopscotch_map<variable, cvc::Term> vars;
    tsl::hopscotch_map<proposition, cvc::Term> props;
    tsl::hopscotch_map<cvc5_decl_key, cvc::Term> decls;

    cvc::Term to_cvc5(
      formula, tsl::hopscotch_map<variable, cvc::Term> const&env
    );
    cvc::Term to_cvc5(
      term, tsl::hopscotch_map<variable, cvc::Term> const&env
    );
    cvc::Sort to_cvc5(sort);

    cvc::Term to_cvc5_func_decl(
      alphabet *sigma, std::string const&name, unsigned arity, bool is_relation
    );
  };

  cvc5::cvc5() : _data{std::make_unique<_cvc5_t>()}
  {
    _data->solver.setLogic("ALL");
    _data->solver.setOption("produce-models", "true");
    _data->solver.setOption("finite-model-find", "true");
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
    _data->sat_response = res.isSat();

    return res.isSat() ? tribool{true} :
           res.isUnsat() ? tribool{false} :
           tribool::undef; // LCOV_EXCL_LINE
  }

  tribool cvc5::is_sat() 
  {
    cvc::Result res = _data->solver.checkSat();
    _data->sat_response = res.isSat();

    return res.isSat() ? tribool{true} :
           res.isUnsat() ? tribool{false} :
           tribool::undef;
  }

  tribool cvc5::value(proposition p) const 
  {
    if(!_data->sat_response)
      return tribool::undef;

    auto it = _data->props.find(p);
    if(it == _data->props.end())
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

  cvc::Sort cvc5::_cvc5_t::to_cvc5(sort s) {
    return s.match(
      [&](integer_sort) {
        return solver.getIntegerSort();
      },
      [&](real_sort) {
        return solver.getRealSort();
      },
      [](otherwise) -> cvc::Sort {
        black_unreachable();
      }
    );
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5(
    formula f, tsl::hopscotch_map<variable, cvc::Term> const&env
  ) {
    return f.match(
      [&](boolean b) { // LCOV_EXCL_LINE
        return b.value() ? solver.mkTrue() : solver.mkFalse();
      },
      [&](atom a) -> cvc::Term { // LCOV_EXCL_LINE
        std::vector<cvc::Term> cvc_terms;
        for(term t : a.terms())
          cvc_terms.push_back(to_cvc5(t, env));

        cvc::Term rel = 
          to_cvc5_func_decl(
            a.sigma(), to_string(a.rel().name()), 
            unsigned(cvc_terms.size()), true);
        
        cvc_terms.insert(cvc_terms.begin(), rel);
        return solver.mkTerm(cvc::APPLY_UF, cvc_terms);
      },
      [&](comparison c, auto left, auto right) {
        std::vector<cvc::Term> terms = { 
          to_cvc5(left, env), to_cvc5(right, env) 
        };
        
        return c.match(
          [&](equal) { 
            return solver.mkTerm(cvc::EQUAL, terms);
          },
          [&](not_equal) { 
            return solver.mkTerm(cvc::NOT,
              {solver.mkTerm(cvc::EQUAL, terms)}
            );
          },
          [&](less_than) { 
            return solver.mkTerm(cvc::LT, terms);
          },
          [&](less_than_equal) { 
            return solver.mkTerm(cvc::LEQ, terms);
          },
          [&](greater_than) { 
            return solver.mkTerm(cvc::GT, terms);
          },
          [&](greater_than_equal) { 
            return solver.mkTerm(cvc::GEQ, terms);
          }
        );
      },
      [&](quantifier q) { // LCOV_EXCL_LINE
        cvc::Term var = solver.mkVar(
          to_cvc5(q.sigma()->default_sort()), to_string(q.var().unique_id())
        );
        
        cvc::Term varlist = solver.mkTerm(cvc::VARIABLE_LIST, {var});

        tsl::hopscotch_map<variable, cvc::Term> new_env = env;
        new_env.insert({q.var(), var});

        if(q.node_type() == quantifier::type::forall{})
          return 
            solver.mkTerm(cvc::FORALL, {varlist, to_cvc5(q.matrix(), new_env)});
        return 
            solver.mkTerm(cvc::EXISTS, {varlist, to_cvc5(q.matrix(), new_env)});
      },
      [&](proposition p) {
        if(auto it = props.find(p); it != props.end())
          return it->second;

        cvc::Term term =
          solver.mkConst(solver.getBooleanSort(), to_string(p.unique_id()));
        props.insert({p, term});

        return term;
      },
      [&](negation, formula n) {
        return solver.mkTerm(cvc::NOT, {to_cvc5(n, env)});
      },
      [&](conjunction c) { // LCOV_EXCL_LINE
        std::vector<cvc::Term> args;
        for(formula op : c.operands())
          args.push_back(to_cvc5(op, env));

        return solver.mkTerm(cvc::AND, args);
      },
      [&](disjunction c) { // LCOV_EXCL_LINE
        std::vector<cvc::Term> args;
        for(formula op : c.operands())
          args.push_back(to_cvc5(op, env));

        return solver.mkTerm(cvc::OR, args);
      },
      [&](implication, formula left, formula right) { // LCOV_EXCL_LINE
        return 
          solver.mkTerm(cvc::IMPLIES,{to_cvc5(left, env), to_cvc5(right, env)});
      },
      [&](iff, formula left, formula right) { // LCOV_EXCL_LINE
        return 
          solver.mkTerm(cvc::EQUAL, {to_cvc5(left, env), to_cvc5(right, env)});
      }
    );
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5_func_decl(
    alphabet *sigma, std::string const&name, unsigned arity, bool is_relation
  ) {
    black_assert(arity > 0);

    if(auto it = decls.find({name, arity, is_relation}); it != decls.end())
      return it->second;

    std::vector<cvc::Sort> sorts{arity};
    std::fill(sorts.begin(), sorts.end(), to_cvc5(sigma->default_sort()));

    cvc::Sort range = is_relation ? solver.getBooleanSort() : sorts[0];

    cvc::Sort funcSort = solver.mkFunctionSort(sorts, range);

    cvc::Term term = solver.mkConst(funcSort, name);
    decls.insert({{name, arity, is_relation}, term});
    return term;
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5(
    term t, tsl::hopscotch_map<variable, cvc::Term> const&env
  ) {
    alphabet *sigma = t.sigma();
    return t.match(
      [&](constant, auto num) { // LCOV_EXCL_LINE
        return num.match(
          [&](zero) { return to_cvc5(constant(sigma->integer(0)), env); },
          [&](one)  { return to_cvc5(constant(sigma->integer(1)), env); },
          [&](integer, int64_t value) {
            if(sigma->default_sort().is<integer_sort>())
              return solver.mkInteger(value);
            return solver.mkReal(value);
          },
          [&](real, double value) {
            auto [num,denum] = 
              black_internal::double_to_fraction(value);
            return solver.mkReal(num, denum);
          }
        );
      },
      [&](variable v) { // LCOV_EXCL_LINE
        if(auto it = env.find(v); it != env.end())
          return it->second;

        if(auto it = vars.find(v); it != vars.end())
          return it->second;

        sort s = t.sigma()->default_sort();
        cvc::Term term = solver.mkConst(to_cvc5(s), to_string(v.unique_id()));
        vars.insert({v, term});
        return term;
      },
      [&](application a) { // LCOV_EXCL_LINE
        std::vector<cvc::Term> cvc_terms;
        for(term t2 : a.terms())
          cvc_terms.push_back(to_cvc5(t2, env));

        cvc::Term func = to_cvc5_func_decl(
          a.sigma(), to_string(a.func().name()), 
          unsigned(cvc_terms.size()), false);

        cvc_terms.insert(cvc_terms.begin(), func);
        return solver.mkTerm(cvc::APPLY_UF, cvc_terms);
      },
      [&](unary_term u) {
        return u.match(
          [&](negative, auto arg) {
            return solver.mkTerm(cvc::NEG, {to_cvc5(arg, env)});
          }
        );
      },
      [&](binary_term b, auto left, auto right) {
        std::vector<cvc::Term> terms = { 
          to_cvc5(left, env), to_cvc5(right, env)
        };

        return b.match(
          [&](subtraction) {
            return solver.mkTerm(cvc::SUB, terms);
          },
          [&](addition) {
            return solver.mkTerm(cvc::ADD, terms);
          },
          [&](multiplication) {
            return solver.mkTerm(cvc::MULT, terms);
          },
          [&](division) {
            return solver.mkTerm(cvc::DIVISION, terms);
          }
        );
      }
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
