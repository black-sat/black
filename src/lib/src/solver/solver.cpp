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

#include <numeric>

namespace black_internal::solver
{
  /*
   * Private implementation of the solver class.
   */
  struct solver::_solver_t 
  {
    // whether a model has been found 
    // i.e., whether solve() has been called and returned true
    bool model = false;

    // alphabet of last solved formula
    alphabet *sigma = nullptr;

    // size of the found model (if any)
    size_t model_size = 0;

    // value for solver::last_bound() 
    size_t last_bound = 0;

    // current SAT solver instance
    std::unique_ptr<black::sat::solver> sat;

    // the name of the currently chosen sat backend
    std::string sat_backend = BLACK_DEFAULT_BACKEND; // sensible default

    std::function<void(trace_t)> tracer = [](trace_t){};

    void trace(size_t k);
    void trace(trace_t::type_t, scope const&, logic::formula<logic::LTLPFO>);
    void trace(trace_t::type_t, scope const&, logic::formula<logic::FO>);

    // Main algorithm
    tribool solve(
      scope const& xi, logic::formula<logic::LTLPFO> f, 
      bool finite, size_t k_max, bool semi_decision
    );
  };

  solver::solver() : _data{std::make_unique<_solver_t>()} { }
  solver::~solver() = default;

  tribool solver::solve(
    scope const& xi, logic::formula<logic::LTLPFO> f, 
    bool finite, size_t k_max, bool semi_decision
  ) {
    return _data->solve(xi, f, finite, k_max, semi_decision);
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

  void solver::set_tracer(std::function<void(trace_t)> const&tracer) {
    _data->tracer = tracer;
  }

  size_t model::size() const {
    return _solver._data->model_size;
  }

  size_t model::loop() const {
    using black_internal::encoder::encoder;
    black_assert(size() > 0);
    
    size_t k = size() - 1;
    for(size_t l = 0; l < k; ++l) {
      proposition loop_prop = encoder::loop_prop(_solver._data->sigma, l, k);
      tribool value = _solver._data->sat->value(loop_prop);
      
      if(value == true)
        return l + 1;
    }

    return size();
  }

  tribool model::value(proposition a, size_t t) const {
    using black_internal::encoder::encoder;
    proposition u = encoder::ground(a, t);

    return _solver._data->sat->value(u);
  }

  void solver::_solver_t::trace(size_t k){
    tracer({nullptr, trace_t::stage, {k}});
  }

  void solver::_solver_t::trace(
    trace_t::type_t type, scope const& xi, logic::formula<logic::LTLPFO> f
  ){
    tracer({&xi, type, {f}});
  }
  
  void solver::_solver_t::trace(
    trace_t::type_t type, scope const& xi, logic::formula<logic::FO> f
  ){
    tracer({&xi, type, {f}});
  }

  /*
   * Main algorithm. Solve the formula with up to `k_max' iterations.
   * If semi_decision = true, we disable the PRUNE rule.
   */
  tribool solver::_solver_t::solve(
    scope const& s, logic::formula<logic::LTLPFO> f, bool finite, 
    size_t k_max, bool semi_decision
  ) {
    scope xi = chain(s);
    
    encoder::encoder enc{f, xi, finite};
    sat = black::sat::solver::get_solver(sat_backend, xi);

    trace(trace_t::nnf, xi, enc.get_formula());

    sigma = f.sigma();
    model = false;
    model_size = 0;
    last_bound = 0;
    for(size_t k = 0; k <= k_max; last_bound = k++)
    {
      trace(k);
      // Generating the k-unraveling.
      // If it is UNSAT, then stop with UNSAT
      auto unrav = enc.k_unraveling(k);
      trace(trace_t::unrav, xi, unrav);
      sat->assert_formula(unrav);
      if(tribool res = sat->is_sat(); !res)
        return res;

      // else, continue to check EMPTY and LOOP.
      // If the k-unrav is SAT assuming EMPTY or LOOP, then stop with SAT
      auto empty = enc.k_empty(k);
      auto loop = enc.k_loop(k);
      trace(trace_t::empty, xi, empty);
      trace(trace_t::loop, xi, loop);
      if(sat->is_sat_with(empty || loop)) {
        model_size = k + 1;
        model = true;
        
        return true;
      }

      // else, generate the PRUNE
      // If the PRUNE is UNSAT, the formula is UNSAT
      if(!semi_decision) {
        auto prune = enc.prune(k);
        trace(trace_t::prune, xi, prune);
        sat->assert_formula(!prune);
        if(tribool res = sat->is_sat(); !res)
          return res;
      }
    } // end for

    return tribool::undef;
  }

  struct check_result_t {
    bool error = false;
    bool has_next = false;

    check_result_t() = default;
    check_result_t(bool b, bool _has_next = false) 
      : error{b}, has_next{_has_next} { }
  };

  static check_result_t operator||(check_result_t r1, check_result_t r2) {
    return {
      r1.error || r2.error,
      r1.has_next || r2.has_next
    };
  }

  // function full of GCOV false negatives
  static check_result_t _check_syntax(
    term t, std::function<void(std::string)> const& err,
    std::vector<variable> const& scope
  ) {
    return t.match( // LCOV_EXCL_LINE
      [](constant) -> check_result_t { return false; },
      [](variable) -> check_result_t { return false; },
      [&](application a) -> check_result_t {
        check_result_t res;
        for(term arg : a.terms())
          res = res || _check_syntax(arg, err, scope);

        return res;
      }, // LCOV_EXCL_LINE
      [&](negative, auto arg) -> check_result_t {
        return _check_syntax(arg, err, scope);
      },
      [&](binary_term, auto left, auto right) {
        return _check_syntax(left, err, scope) ||
               _check_syntax(right, err, scope);
      },
      [&](to_integer, auto arg) {
        return _check_syntax(arg, err, scope);
      },
      [&](to_real, auto arg) {
        return _check_syntax(arg, err, scope);
      },
      [&](unary_term, term arg) -> check_result_t {
        if(!arg.is<variable>()) {
          err(
            "next()/wnext()/prev()/wprev() terms can only be applied "
            "directly to variables"
          );
          return true;
        }
        
        return {false, true};
      }
    );
  }

  // function full of GCOV false negatives
  static check_result_t _check_syntax(
    formula f, std::function<void(std::string)> const& err, 
    std::vector<variable> const& scope
  ) {
    return f.match( // LCOV_EXCL_LINE
      [](boolean) -> check_result_t { return false; },
      [](proposition) -> check_result_t { return false; },
      [&](atom a) -> check_result_t {  
        check_result_t res;
        for(term t : a.terms())
          res = res || _check_syntax(t, err, scope);
        
        return res;  
      }, // LCOV_EXCL_LINE
      [&](comparison, auto left, auto right) {
        return _check_syntax(left, err, scope) ||
               _check_syntax(right, err, scope);
      },
      [&](quantifier q) -> check_result_t {
        std::vector<variable> new_scope = scope;
        new_scope.push_back(q.decl().variable());
        
        return _check_syntax(q.matrix(), err, new_scope);
      }, // LCOV_EXCL_LINE
      [&](negation, auto arg) {
        return _check_syntax(arg, err, scope);
      },
      [&](disjunction o) {
        check_result_t r{false, false};
        std::vector<check_result_t> results;
        for(auto op : o.operands()) {
          results.push_back(
            _check_syntax(op, err, scope)
          );
        }
        return std::accumulate(
          begin(results), end(results), r, std::logical_or<>{}
        );
      },
      [&](conjunction c) {
        check_result_t r{false, false};
        std::vector<check_result_t> results;
        for(auto op : c.operands()) {
          results.push_back(
            _check_syntax(op, err, scope)
          );
        }
        return std::accumulate(
          begin(results), end(results), r, std::logical_or<>{}
        );
      },
      [&](implication, formula left, formula right) {
        return _check_syntax(!left || right, err, scope);
      },
      [&](iff, formula left, formula right) {
        return _check_syntax( // LCOV_EXCL_LINE
          implies(left, right) && implies(right, left), 
          err, scope
        );
      },
      [&](tomorrow, auto arg) {
        return _check_syntax(arg, err, scope);
      },
      [&](w_tomorrow, auto arg) {
        return _check_syntax(arg, err, scope);
      },
      [&](yesterday, auto arg) {
        return _check_syntax(arg, err, scope);
      },
      [&](w_yesterday, auto arg) {
        return _check_syntax(arg, err, scope);
      },
      [&](only<temporal> t) -> check_result_t {
        if(!scope.empty()) {
          err(
            "Temporal operators (excepting X/wX/Y/Z) cannot appear "
            "inside quantifiers"
          );
          return true;
        }

        return t.match( // LCOV_EXCL_LINE
          [&](unary, formula arg) {
            return _check_syntax(arg, err, scope);
          },
          [&](binary, formula left, formula right) {
            return _check_syntax(left, err, scope) || 
                   _check_syntax(right, err, scope);
          }
        );
      }
    );
  }

  bool 
  solver::check_syntax(formula f, std::function<void(std::string)> const&err) {
    tsl::hopscotch_map<identifier, size_t> rels;
    tsl::hopscotch_map<identifier, size_t> funcs;
    return 
      !_check_syntax(f, err, std::vector<variable>{}).error;
  }

  

} // end namespace black::size_ternal
