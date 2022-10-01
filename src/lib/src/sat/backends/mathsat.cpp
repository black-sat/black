//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Luca Geatti
// (C) 2019 Nicola Gigante
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


#include <black/sat/backends/mathsat.hpp>

#include <black/logic/logic.hpp>
#include <black/logic/prettyprint.hpp>

#include <mathsat.h>
#include <fmt/format.h>
#include <tsl/hopscotch_map.h>

#include <string>

BLACK_REGISTER_SAT_BACKEND(mathsat, {black::sat::feature::smt})

namespace black_internal::mathsat
{
  struct mathsat::_mathsat_t {
    msat_env env;
    tsl::hopscotch_map<formula, msat_term> formulas;
    tsl::hopscotch_map<term, msat_term> terms;
    tsl::hopscotch_map<function, msat_decl> functions;
    tsl::hopscotch_map<relation, msat_decl> relations;
    tsl::hopscotch_map<term, msat_decl> variables;
    std::optional<msat_model> model;

    msat_term to_mathsat(formula);
    msat_term to_mathsat_inner(formula);
    msat_term to_mathsat(term);
    msat_term to_mathsat_inner(term);
    msat_type to_mathsat(sort);
    msat_decl to_mathsat(function);
    msat_decl to_mathsat(relation);
    msat_decl to_mathsat(variable);

    ~_mathsat_t() {
      if(model)
        msat_destroy_model(*model);
    }
  };

  mathsat::mathsat() : _data{std::make_unique<_mathsat_t>()}
  {  
    msat_config cfg = msat_create_config();
    msat_set_option(cfg, "model_generation", "true");
    msat_set_option(cfg, "unsat_core_generation","3");
    
    _data->env = msat_create_env(cfg);
  }

  mathsat::~mathsat() { }

  void mathsat::assert_formula(formula f) {
    msat_assert_formula(_data->env, _data->to_mathsat(f));
  }

  tribool mathsat::is_sat() { 
    msat_result res = msat_solve(_data->env);

    if(res == MSAT_SAT) {
      if(_data->model)
        msat_destroy_model(*_data->model);

      _data->model = msat_get_model(_data->env);
      black_assert(!MSAT_ERROR_MODEL(*_data->model));
    }

    return res == MSAT_SAT ? tribool{true} :
           res == MSAT_UNSAT ? tribool{false} :
           tribool::undef; // LCOV_EXCL_LINE
  }

  tribool mathsat::is_sat_with(formula f) 
  {
    msat_push_backtrack_point(_data->env);
  
    assert_formula(f);
    msat_result res = msat_solve(_data->env);

    if(res == MSAT_SAT) {
      if(_data->model)
        msat_destroy_model(*_data->model);

      _data->model = msat_get_model(_data->env);
      black_assert(!MSAT_ERROR_MODEL(*_data->model));
    }
  
    msat_pop_backtrack_point(_data->env);
    return res == MSAT_SAT ? tribool{true} :
           res == MSAT_UNSAT ? tribool{false} :
           tribool::undef; // LCOV_EXCL_LINE
  }

  tribool mathsat::value(proposition a) const {
    auto it = _data->formulas.find(a);
    if(it == _data->formulas.end())
      return tribool::undef;

    if(!_data->model)
      return tribool::undef;
    
    msat_term var = it->second;
    msat_term result = msat_model_eval(*_data->model, var);
    if(msat_term_is_true(_data->env, result))
      return true;
    if(msat_term_is_false(_data->env, result))
      return false;

    return tribool::undef; // LCOV_EXCL_LINE 
  }

  void mathsat::clear() {
    msat_reset_env(_data->env);
  }

  msat_term mathsat::_mathsat_t::to_mathsat(formula f) 
  {
    if(auto it = formulas.find(f); it != formulas.end()) 
      return it->second;

    msat_term term = to_mathsat_inner(f);
    formulas.insert({f, term});

    return term;
  }

  msat_term mathsat::_mathsat_t::to_mathsat_inner(formula f) 
  {
    return f.match(
      [this](boolean b) { // LCOV_EXCL_LINE
        return b.value() ? 
          msat_make_true(env) : msat_make_false(env);
      },
      [this](atom a) { // LCOV_EXCL_LINE
        std::vector<msat_term> args;
        for(term t : a.terms())
          args.push_back(to_mathsat(t));
        
        msat_decl rel = to_mathsat(a.rel());

        return msat_make_term(env, rel, args.data());
      },
      [&](comparison c, auto left, auto right) {
        return c.match(
          [&](equal) {
            return msat_make_eq(env, to_mathsat(left), to_mathsat(right));
          },
          [&](not_equal) {
            return msat_make_not(env, 
              msat_make_eq(env, to_mathsat(left), to_mathsat(right)));
          },
          [&](less_than) {
            return msat_make_and(env,
              msat_make_leq(env, to_mathsat(left), to_mathsat(right)),
              msat_make_not(env, 
                msat_make_eq(env, to_mathsat(left), to_mathsat(right)))
            );
          },
          [&](less_than_equal) {
            return msat_make_leq(env, to_mathsat(left), to_mathsat(right));
          },
          [&](greater_than) {
            return msat_make_not(env, 
              msat_make_leq(env, to_mathsat(left), to_mathsat(right)));
          },
          [&](greater_than_equal) {
            return msat_make_or(env,
              msat_make_eq(env, to_mathsat(left), to_mathsat(right)),
              msat_make_not(env,
                 msat_make_leq(env, to_mathsat(left), to_mathsat(right)))
            );
          }
        );
      },
      // mathsat does not support quantifiers
      [](quantifier) -> msat_term { black_unreachable(); }, // LCOV_EXCL_LINE
      [this](proposition p) { // LCOV_EXCL_LINE
        msat_decl msat_prop =
          msat_declare_function(env, to_string(p.unique_id()).c_str(),
          msat_get_bool_type(env));

        return msat_make_constant(env, msat_prop);
      },
      [this](negation n) { // LCOV_EXCL_LINE
        return msat_make_not(env, to_mathsat(n.argument()));
      },
      [this](conjunction c) { // LCOV_EXCL_LINE
        msat_term acc = msat_make_true(env);

        for(formula op : c.operands())
          acc = msat_make_and(env, acc, to_mathsat(op));

        return acc;
      },
      [this](disjunction c) { // LCOV_EXCL_LINE
        msat_term acc = msat_make_false(env);

        for(formula op : c.operands())
          acc = msat_make_or(env, acc, to_mathsat(op));

        return acc;
      },
      [this](implication t) { // LCOV_EXCL_LINE
        return
          msat_make_or(env,
            msat_make_not(env, 
              to_mathsat(t.left())), to_mathsat(t.right())
          );
      },
      [this](iff i) {
        return msat_make_iff(env, 
          to_mathsat(i.left()), to_mathsat(i.right()));
      }
    );
  }

  msat_type mathsat::_mathsat_t::to_mathsat(sort s) {
    return s.match(
      [&](integer_sort) {
        return msat_get_integer_type(env);
      },
      [&](real_sort) {
        return msat_get_rational_type(env);
      },
      [](otherwise) -> msat_type { black_unreachable(); } // LCOV_EXCL_LINE
    );
  }

  msat_decl mathsat::_mathsat_t::to_mathsat(function f) {
    if(auto it = functions.find(f); it != functions.end())
      return it->second; // LCOV_EXCL_LINE

    size_t arity = f.signature().size();
    std::vector<msat_type> types;
    for(size_t i = 0; i < arity; ++i)
      types.push_back(to_mathsat(f.signature()[i]));

    msat_type functype = msat_get_function_type(
      env, types.data(), arity, to_mathsat(f.result())
    ); // LCOV_EXCL_LINE
    msat_decl d = 
      msat_declare_function(env, to_string(f.unique_id()).c_str(), functype);

    functions.insert({f, d});

    return d;
  }

  msat_decl mathsat::_mathsat_t::to_mathsat(relation r) {
    if(auto it = relations.find(r); it != relations.end())
      return it->second; // LCOV_EXCL_LINE

    msat_type bool_type = msat_get_bool_type(env);

    size_t arity = r.signature().size();
    std::vector<msat_type> types;
    for(size_t i = 0; i < arity; ++i)
      types.push_back(to_mathsat(r.signature()[i]));

    msat_type functype = msat_get_function_type(
      env, types.data(), arity, bool_type
    ); // LCOV_EXCL_LINE
    msat_decl d = 
      msat_declare_function(env, to_string(r.unique_id()).c_str(), functype);

    relations.insert({r, d});

    return d;
  }

  msat_decl mathsat::_mathsat_t::to_mathsat(variable x) {
    if(auto it = variables.find(x); it != variables.end())
      return it->second; // LCOV_EXCL_LINE

    msat_decl var = 
      msat_declare_function(env, to_string(x).c_str(), to_mathsat(x.sort()));

    return var;
  }

  msat_term mathsat::_mathsat_t::to_mathsat(term t) 
  {
    if(auto it = terms.find(t); it != terms.end()) 
      return it->second;

    msat_term term = to_mathsat_inner(t);
    terms.insert({t, term});

    return term;
  }
  
  msat_term mathsat::_mathsat_t::to_mathsat_inner(term t) {
    return t.match(
      [&](constant, auto num) { // LCOV_EXCL_LINE
        return num.match(
          [&](integer, int64_t value) {
            return msat_make_number(env, std::to_string(value).c_str());
          },
          [&](real, double value) {
            return msat_make_number(env, std::to_string(value).c_str());
          }
        );
      },
      [&](variable x) { // LCOV_EXCL_LINE
        return msat_make_constant(env, to_mathsat(x));
      },
      [&](application a) {
        std::vector<msat_term> args;
        for(term t2 : a.terms())
          args.push_back(to_mathsat(t2));

        black_assert(a.terms().size() > 0);

        msat_decl func = to_mathsat(a.func());
        return msat_make_uf(env, func, args.data());
      }, // LCOV_EXCL_LINE
      [&](unary_term u, auto arg) {
        return u.match(
          [&](negative) {
            return msat_make_times(env, 
              msat_make_number(env, "-1"), to_mathsat(arg));
          },
          [&](to_integer) {
            return to_mathsat(arg);
          },
          [&](to_real) {
            return to_mathsat(arg);
          }
        );
      },
      [&](binary_term b, auto left, auto right) {
        return b.match(
          [&](subtraction) {
            return msat_make_plus(env, 
              to_mathsat(left),
              msat_make_times(env, 
                msat_make_number(env, "-1"), to_mathsat(right))
            );
          },
          [&](addition) {
            return 
              msat_make_plus(env, to_mathsat(left), to_mathsat(right));
          },
          [&](multiplication) {
            return 
              msat_make_times(env, to_mathsat(left), to_mathsat(right));
          },
          [&](division) {
            return msat_make_divide(env, to_mathsat(left), to_mathsat(right));
          },
          [&](int_division) {
            return msat_make_divide(env, to_mathsat(left), to_mathsat(right));
          }
        );
      }
    );
  }

  std::optional<std::string> mathsat::license() const {
    return R"(
MathSAT5 is copyrighted 2009-2020 by Fondazione Bruno Kessler, Trento, Italy, 
University of Trento, Italy, and others. All rights reserved.

MathSAT5 is available for research and evaluation purposes only. It can not be 
used in a commercial environment, particularly as part of a commercial product, 
without written permission. MathSAT5 is provided as is, without any warranty.
)";
  }

}
