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

#include <black/logic/alphabet.hpp>
#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>

#include <mathsat.h>
#include <fmt/format.h>
#include <tsl/hopscotch_map.h>

#include <string>

BLACK_REGISTER_SAT_BACKEND(mathsat)

namespace black::sat::backends
{
  struct mathsat::_mathsat_t {
    msat_env env;
    tsl::hopscotch_map<formula, msat_term> terms;
    std::optional<msat_model> model;

    msat_term to_mathsat(formula);
    msat_term to_mathsat_inner(formula);

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

  bool mathsat::is_sat() { 
    msat_result res = msat_solve(_data->env);

    if(res == MSAT_SAT) {
      if(_data->model)
        msat_destroy_model(*_data->model);

      _data->model = msat_get_model(_data->env);
      black_assert(!MSAT_ERROR_MODEL(*_data->model));
    }

    return (res == MSAT_SAT);
  }

  bool mathsat::is_sat_with(formula f) 
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
    return (res == MSAT_SAT);
  }

  tribool mathsat::value(proposition a) const {
    auto it = _data->terms.find(a);
    if(it == _data->terms.end())
      return tribool::undef;

    if(!_data->model)
      return tribool::undef;
    
    msat_term var = it->second;
    msat_term result = msat_model_eval(*_data->model, var);
    if(msat_term_is_true(_data->env, result))
      return true;
    if(msat_term_is_false(_data->env, result))
      return false;

    return tribool::undef;
  }

  void mathsat::clear() {
    msat_reset_env(_data->env);
  }

  msat_term mathsat::_mathsat_t::to_mathsat(formula f) 
  {
    if(auto it = terms.find(f); it != terms.end()) 
      return it->second;

    msat_term term = to_mathsat_inner(f);
    terms.insert({f, term});

    return term;
  }

  msat_term mathsat::_mathsat_t::to_mathsat_inner(formula f) 
  {
    return f.match(
      [this](boolean b) {
        return b.value() ? 
          msat_make_true(env) : msat_make_false(env);
      },
      [this](atom a) {
        black_unreachable();
      },
      [this](proposition p) {
        msat_decl msat_prop =
          msat_declare_function(env, to_string(p.unique_id()).c_str(),
          msat_get_bool_type(env));

        return msat_make_constant(env, msat_prop);
      },
      [this](negation n) {
        return msat_make_not(env, to_mathsat(n.operand()));
      },
      [this](big_conjunction c) {
        msat_term acc = msat_make_true(env);

        for(formula op : c.operands())
          acc = msat_make_and(env, acc, to_mathsat(op));

        return acc;
      },
      [this](big_disjunction c) {
        msat_term acc = msat_make_false(env);

        for(formula op : c.operands())
          acc = msat_make_or(env, acc, to_mathsat(op));

        return acc;
      },
      [this](implication t) {
        return
          msat_make_or(env,
            msat_make_not(env, 
              to_mathsat(t.left())), to_mathsat(t.right())
          );
      },
      [this](iff i) {
        return msat_make_iff(env, 
          to_mathsat(i.left()), to_mathsat(i.right()));
      },
      [](temporal) -> msat_term { // LCOV_EXCL_LINE
        black_unreachable(); // LCOV_EXCL_LINE
      }
    );
  }

  bool mathsat::supports_logic(logic ) const {
    return true; // TODO: check if the actual theory is supported
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
