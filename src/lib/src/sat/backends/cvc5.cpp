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

  template<typename T>
  std::string unique_symbol(T v) {
    std::string name = to_string(v.name());
    if(name.size() >= 20)
      return to_string(v.unique_id());
      
    return to_string(v.unique_id()) + ":" + to_string(v.name());
  }

  namespace cvc = ::cvc5;
  struct cvc5::_cvc5_t {
    class scope global_xi;
    class scope xi;

    _cvc5_t(logic::scope const& _xi) 
      : global_xi{chain(_xi)}, xi{chain(global_xi)} { }

    cvc::Solver solver;
    bool sat_response = false;

    tsl::hopscotch_map<proposition, cvc::Term> props;

    // keep track of things to dump in dump_model()
    std::vector<cvc::Sort> dumped_sorts;
    std::vector<cvc::Term> dumped_terms;

    cvc::Term to_cvc5(formula);
    cvc::Term to_cvc5(term);
    cvc::Term to_cvc5(function);
    cvc::Term to_cvc5(relation);
    cvc::Sort to_cvc5(std::optional<sort>);

  };

  cvc5::cvc5(class scope const&xi) : _data{std::make_unique<_cvc5_t>(xi)}
  {
    _data->solver.setLogic("ALL");
    _data->solver.setOption("produce-models", "true");
    _data->solver.setOption("finite-model-find", "true");
  }

  cvc5::~cvc5() = default;

  void cvc5::assert_formula(formula f) {
    cvc::Term term = _data->to_cvc5(f);
    _data->solver.assertFormula(term);
  }

  tribool cvc5::is_sat_with(formula f) 
  {
    cvc::Term term = _data->to_cvc5(f);

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
      return tribool::undef; // LCOV_EXCL_LINE

    return res.getBooleanValue();
  }

  tribool cvc5::value(atom a) const 
  {
    if(!_data->sat_response)
      return tribool::undef;
    
    cvc::Term term = _data->to_cvc5(a);
    cvc::Term res = _data->solver.getValue(term);

    if(res.isNull())
      return tribool::undef; // LCOV_EXCL_LINE

    return res.getBooleanValue();
  }

  std::string cvc5::dump_model() const {
    if(!_data->sat_response)
      return "<no model>";
    
    return _data->solver.getModel(_data->dumped_sorts, _data->dumped_terms);
  }

  void cvc5::clear() {
    _data->solver.resetAssertions();
  }

  cvc::Sort cvc5::_cvc5_t::to_cvc5(std::optional<sort> s) {
    black_assert(s.has_value());
    
    if(auto cvcSort = global_xi.data<cvc::Sort>(*s); cvcSort.has_value())
      return *cvcSort;

    cvc::Sort result = s->match(
      [&](integer_sort) {
        return solver.getIntegerSort();
      },
      [&](real_sort) {
        return solver.getRealSort();
      },
      [&](named_sort n) {
        auto d = global_xi.domain(n);
        if(!d) {
          auto us = solver.mkUninterpretedSort(unique_symbol(n));
          dumped_sorts.push_back(us);
          return us;
        }
        
        auto decl = solver.mkDatatypeDecl(unique_symbol(n));
        
        for(auto x : d->elements()) {
          auto ctor = 
            solver.mkDatatypeConstructorDecl(unique_symbol(x));
          decl.addConstructor(ctor);
        }

        return solver.mkDatatypeSort(decl);
      }
    );

    global_xi.set_data(*s, result);
    return result;
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5(formula f) {
    return f.match(
      [&](boolean b) { // LCOV_EXCL_LINE
        return b.value() ? solver.mkTrue() : solver.mkFalse();
      },
      [&](atom a) -> cvc::Term { // LCOV_EXCL_LINE
        std::vector<cvc::Term> cvc_terms;
        cvc_terms.push_back(to_cvc5(a.rel()));

        for(term t : a.terms())
          cvc_terms.push_back(to_cvc5(t));

        return solver.mkTerm(cvc::APPLY_UF, cvc_terms);
      },
      [&](equality e, auto args) {
        std::vector<cvc::Term> terms;
        for(auto t : args)
          terms.push_back(to_cvc5(t));

        return e.match(
          [&](equal) { 
            return solver.mkTerm(cvc::EQUAL, terms);
          },
          [&](distinct) {
            return solver.mkTerm(cvc::DISTINCT, terms);
          }
        );
      },
      [&](comparison c, auto left, auto right) {
        std::vector<cvc::Term> terms = { to_cvc5(left), to_cvc5(right) };
        
        return c.match(
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
        logic::nest_scope_t nest{xi};
        
        std::vector<cvc::Term> vars;
        for(auto decl : q.variables()) {
          xi.declare(decl);
          cvc::Term var = solver.mkVar(
            to_cvc5(decl.sort()), unique_symbol(decl.variable())
          );
          xi.set_data(decl.variable(), var);
          vars.push_back(var);
        }

        cvc::Term cvc5matrix = to_cvc5(q.matrix());
        cvc::Term varlist = solver.mkTerm(cvc::VARIABLE_LIST, vars);

        if(q.node_type() == quantifier::type::forall{})
          return 
            solver.mkTerm(cvc::FORALL, {varlist, cvc5matrix});
        return 
            solver.mkTerm(cvc::EXISTS, {varlist, cvc5matrix});
      },
      [&](proposition p) {
        if(auto it = props.find(p); it != props.end())
          return it->second;

        cvc::Term term =
          solver.mkConst(solver.getBooleanSort(), unique_symbol(p));
        props.insert({p, term});
        dumped_terms.push_back(term);

        return term;
      },
      [&](negation, formula n) {
        return solver.mkTerm(cvc::NOT, {to_cvc5(n)});
      },
      [&](conjunction c) { // LCOV_EXCL_LINE
        std::vector<cvc::Term> args;
        for(formula op : c.operands())
          args.push_back(to_cvc5(op));

        return solver.mkTerm(cvc::AND, args);
      },
      [&](disjunction c) { // LCOV_EXCL_LINE
        std::vector<cvc::Term> args;
        for(formula op : c.operands())
          args.push_back(to_cvc5(op));

        return solver.mkTerm(cvc::OR, args);
      },
      [&](implication, formula left, formula right) { // LCOV_EXCL_LINE
        return 
          solver.mkTerm(cvc::IMPLIES,{to_cvc5(left), to_cvc5(right)});
      },
      [&](iff, formula left, formula right) { // LCOV_EXCL_LINE
        return 
          solver.mkTerm(cvc::EQUAL, {to_cvc5(left), to_cvc5(right)});
      }
    );
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5(function f) {
    if(auto term = xi.data<cvc::Term>(f); term.has_value())
      return *term;

    auto signature = xi.signature(f);
    black_assert(signature.has_value());

    size_t arity = signature->size();
    std::vector<cvc::Sort> fsorts;
    for(size_t i = 0; i < arity; ++i)
      fsorts.push_back(to_cvc5(signature->at(i)));

    cvc::Sort funcSort = solver.mkFunctionSort(fsorts, to_cvc5(xi.sort(f)));

    cvc::Term term = solver.mkConst(funcSort, unique_symbol(f));
    global_xi.set_data(f, term);
    return term;
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5(relation r) {
    if(auto term = xi.data<cvc::Term>(r); term.has_value())
      return *term;

    auto signature = xi.signature(r);
    black_assert(signature.has_value());

    size_t arity = signature->size();
    std::vector<cvc::Sort> rsorts;
    for(size_t i = 0; i < arity; ++i)
      rsorts.push_back(to_cvc5(signature->at(i)));

    cvc::Sort funcSort = solver.mkFunctionSort(rsorts, solver.getBooleanSort());

    cvc::Term term = solver.mkConst(funcSort, unique_symbol(r));
    global_xi.set_data(r, term);
    return term;
  }

  cvc::Term cvc5::_cvc5_t::to_cvc5(term t) {
    return t.match(
      [&](constant, auto n) { // LCOV_EXCL_LINE
        return n.match(
          [&](integer, int64_t value) {
            return solver.mkInteger(value);
          },
          [&](real, double value) {
            auto [num,denum] = 
              black_internal::double_to_fraction(value);
            return solver.mkReal(num, denum);
          }
        );
      },
      [&](variable v) {
        // then we look up the variable in the scope
        if(auto var = xi.data<cvc::Term>(v); var.has_value())
          return *var;

        auto vSort = xi.sort(v); // we look up the sort of the variable
        black_assert(vSort); 
        cvc::Sort cvcSort = to_cvc5(*vSort);

        // if the sort has an enumerated domain, the variable is rendered
        // as a constructor application of the relevant datatype
        if(xi.domain(*vSort)) {
          cvc::Datatype dt = cvcSort.getDatatype();
          auto it = std::find_if(dt.begin(), dt.end(), [&](auto c){
            return c.getName() == unique_symbol(v);
          });
          if(it != dt.end()) {
            cvc::Term term = 
              solver.mkTerm(
                cvc::APPLY_CONSTRUCTOR, {it->getTerm()}
              );
            global_xi.set_data(v, term);
            return term;
          }
        }

        // if not, this is a normal variable
        cvc::Term term = solver.mkConst(cvcSort, unique_symbol(v));
        xi.set_data(v, term);
        dumped_terms.push_back(term);
        return term;
      },
      [&](application a) { // LCOV_EXCL_LINE
        std::vector<cvc::Term> cvc_terms;
        for(term t2 : a.terms())
          cvc_terms.push_back(to_cvc5(t2));

        cvc::Term func = to_cvc5(a.func());

        cvc_terms.insert(cvc_terms.begin(), func);
        return solver.mkTerm(cvc::APPLY_UF, cvc_terms);
      },
      [&](unary_term u) {
        return u.match(
          [&](negative, auto arg) {
            return solver.mkTerm(cvc::NEG, {to_cvc5(arg)});
          },
          [&](to_integer, auto arg) {
            return solver.mkTerm(cvc::TO_INTEGER, {to_cvc5(arg)});
          },
          [&](to_real, auto arg) {
            return solver.mkTerm(cvc::TO_REAL, {to_cvc5(arg)});
          }
        );
      },
      [&](binary_term b, term left, term right) {
        std::vector<cvc::Term> terms = { to_cvc5(left), to_cvc5(right) };

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
          },
          [&](int_division) {
            return solver.mkTerm(cvc::INTS_DIVISION, terms);
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
