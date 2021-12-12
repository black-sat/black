//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Luca Geatti
// (C) 2019 Nicola Gigante
// (C) 2020 Gabriele Venturato
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

#include <black/support/config.hpp>
#include <black/support/range.hpp>
#include <black/solver/solver.hpp>
#include <black/solver/encoding.hpp>
#include <black/sat/solver.hpp>

namespace black::internal
{
  /*
   * Private implementation of the solver class.
   */
  struct solver::_solver_t 
  {
    std::optional<struct encoder> encoder;

    // whether a model has been found 
    // i.e., whether solve() has been called and returned true
    bool model = false;

    // size of the found model (if any)
    size_t model_size = 0;

    // value for solver::last_bound() 
    size_t last_bound = 0;

    // current SAT solver instance
    std::unique_ptr<sat::solver> sat;

    // the name of the currently chosen sat backend
    std::string sat_backend = BLACK_DEFAULT_BACKEND; // sensible default

    // Main algorithm
    tribool solve(size_t k_max, bool semi_decision);
  };

  solver::solver() : _data{std::make_unique<_solver_t>()} { }
  solver::~solver() = default;

  void solver::set_formula(formula f, bool finite) {
    _data->model = false;
    _data->model_size = 0;
    _data->last_bound = 0;
    _data->encoder = encoder{f, finite};
  }

  tribool solver::solve(size_t k_max, bool semi_decision) {
    if(!_data->encoder.has_value())
      return true;
    
    return _data->solve(k_max, semi_decision);
  }

  std::optional<model> solver::model() const {
    if(!_data->model)
      return {};
    
    return {{*this}};
  }

  size_t solver::last_bound() const {
    return _data->last_bound;
  }

  void solver::set_sat_backend(std::string name) {
    _data->sat_backend = std::move(name);
  }

  std::string solver::sat_backend() const {
    return _data->sat_backend;
  }

  size_t model::size() const {
    return _solver._data->model_size;
  }

  size_t model::loop() const {
    black_assert(size() > 0);
    black_assert(_solver._data->encoder);
    
    size_t k = size() - 1;
    for(size_t l = 0; l < k; ++l) {
      proposition loop_prop = _solver._data->encoder->loop_prop(l, k);
      tribool value = _solver._data->sat->value(loop_prop);
      
      if(value == true)
        return l + 1;
    }

    return size();
  }

  tribool model::value(proposition a, size_t t) const {
    black_assert(_solver._data->encoder);
    
    proposition u = _solver._data->encoder->ground(a, t);

    return _solver._data->sat->value(u);
  }

  /*
   * Main algorithm. Solve the formula with up to `k_max' iterations.
   * If semi_decision = true, we disable the PRUNE rule.
   */
  tribool solver::_solver_t::solve(size_t k_max, bool semi_decision)
  {
    if(!encoder)
      return tribool::undef;
    
    sat = sat::solver::get_solver(sat_backend);

    model = false;
    last_bound = 0;
    for(size_t k = 0; k <= k_max; last_bound = k++)
    {
      // Generating the k-unraveling.
      // If it is UNSAT, then stop with UNSAT
      sat->assert_formula(encoder->k_unraveling(k));
      if(tribool res = sat->is_sat(); !res)
        return res;

      // else, continue to check EMPTY and LOOP.
      // If the k-unrav is SAT assuming EMPTY or LOOP, then stop with SAT
      if(sat->is_sat_with(encoder->k_empty(k) || encoder->k_loop(k))) {
        model_size = k + 1;
        model = true;
        
        return true;
      }

      // else, generate the PRUNE
      // If the PRUNE is UNSAT, the formula is UNSAT
      if(!semi_decision) {
        sat->assert_formula(!encoder->prune(k));
        if(tribool res = sat->is_sat(); !res)
          return res;
      }
    } // end for

    return tribool::undef;
  }

  struct check_result_t {
    bool error = false;
    bool has_next = false;
    bool has_disjunctions = false;

    check_result_t() = default;
    check_result_t(bool b, bool _has_next = false, bool _has_disj = false) 
      : error{b}, has_next{_has_next}, has_disjunctions{_has_disj} { }
  };

  static check_result_t operator||(check_result_t r1, check_result_t r2) {
    return {
      r1.error || r2.error,
      r1.has_next || r2.has_next, 
      r1.has_disjunctions || r2.has_disjunctions
    };
  }

  static check_result_t _check_syntax(
    term t, std::function<void(std::string)> const& err,
    std::vector<variable> const& scope
  ) {
    return t.match(
      [](constant) -> check_result_t { return false; },
      [](variable) -> check_result_t { return false; },
      [&](application a) {
        check_result_t res;

        for(term arg : a.arguments())
          res = res || _check_syntax(arg, err, scope);

        return res;
      },
      [&](next n) -> check_result_t {
        term arg = n.argument();
        if(!arg.is<variable>()) {
          err("next() terms can only be applied directly to variables");
          return true;
        }

        for(variable v : scope) {
          if(v == arg) {
            err("next() terms cannot be applied to quantified variables");
            return true;
          }
        }
        
        return {false, true, false};
      },
      [&](wnext n) -> check_result_t {
        term arg = n.argument();
        if(!arg.is<variable>()) {
          err("wnext() terms can only be applied directly to variables");
          return true;
        }

        for(variable v : scope) {
          if(v == arg) {
            err("wnext() terms cannot be applied to quantified variables");
            return true;
          }
        }
        
        return {false, true, false};
      }
    );
  }

  static check_result_t _check_syntax(
    formula f, std::function<void(std::string)> const& err, 
    std::vector<variable> const& scope, bool positive
  ) {
    return f.match(
      [](boolean) -> check_result_t { return false; },
      [](proposition) -> check_result_t { return false; },
      [&](atom a) {
        check_result_t res;

        for(term t : a.terms())
          res = res || _check_syntax(t, err, scope);
        
        return res;  
      },
      [&](quantifier q) -> check_result_t {
        std::vector<variable> s = scope;
        s.push_back(q.var());
        check_result_t res = _check_syntax(q.matrix(), err, s, positive);
        if(res.has_next && res.has_disjunctions) {
          err(
            "next() terms and disjunctions cannot be mixed inside quantifiers"
          );
          return false;
        }

        return res;
      },
      [&](negation, formula arg) {
        return _check_syntax(arg, err, scope, !positive);
      },
      [&](disjunction, formula left, formula right) {
        check_result_t r{false, false, positive};
        return r || _check_syntax(left, err, scope, positive) ||
                    _check_syntax(right, err, scope, positive);
      },
      [&](conjunction, formula left, formula right) {
        check_result_t r{false, false, !positive};
        return r || _check_syntax(left, err, scope, positive) ||
                    _check_syntax(right, err, scope, positive);
      },
      [&](implication, formula left, formula right) {
        return _check_syntax(!left || right, err, scope, positive);
      },
      [&](iff, formula left, formula right) {
        return _check_syntax(
          implies(left, right) && implies(right, left), 
          err, scope, positive
        );
      },
      [&](temporal t) -> check_result_t {
        if(!scope.empty()) {
          err("Temporal operators cannot appear inside quantifiers");
          return true;
        }

        return t.match(
          [&](unary, formula arg) {
            return _check_syntax(arg, err, scope, positive);
          },
          [&](binary, formula left, formula right) {
            return _check_syntax(left, err, scope, positive) || 
                   _check_syntax(right, err, scope, positive);
          }
        );
      }
    );
  }

  bool 
  solver::check_syntax(formula f, std::function<void(std::string)> const&err) {
    return _check_syntax(f, err, std::vector<variable>{}, true).error;
  }

  

} // end namespace black::size_ternal
