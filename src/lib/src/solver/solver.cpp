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

    std::function<void(trace_t)> tracer = [](trace_t){};

    void trace(size_t k);
    void trace(trace_t::type type, formula);

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

  void solver::set_tracer(std::function<void(trace_t)> const&tracer) {
    _data->tracer = tracer;
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

  void solver::_solver_t::trace(size_t k){
    tracer({trace_t::stage, {k}});
  }

  void solver::_solver_t::trace(trace_t::type type, formula f){
    black_assert(type != trace_t::stage);
    tracer({type, {f}});
  }

  /*
   * Main algorithm. Solve the formula with up to `k_max' iterations.
   * If semi_decision = true, we disable the PRUNE rule.
   */
  tribool solver::_solver_t::solve(size_t k_max, bool semi_decision)
  {
    black_assert(encoder); // LCOV_EXCL_LINE
    
    trace(trace_t::nnf, encoder->get_formula());

    sat = sat::solver::get_solver(sat_backend);

    model = false;
    last_bound = 0;
    for(size_t k = 0; k <= k_max; last_bound = k++)
    {
      trace(k);
      // Generating the k-unraveling.
      // If it is UNSAT, then stop with UNSAT
      formula unrav = encoder->k_unraveling(k);
      trace(trace_t::unrav, unrav);
      sat->assert_formula(unrav);
      if(tribool res = sat->is_sat(); !res)
        return res;

      // else, continue to check EMPTY and LOOP.
      // If the k-unrav is SAT assuming EMPTY or LOOP, then stop with SAT
      formula empty = encoder->k_empty(k);
      formula loop = encoder->k_loop(k);
      trace(trace_t::empty, empty);
      trace(trace_t::loop, loop);
      if(sat->is_sat_with(empty || loop)) {
        model_size = k + 1;
        model = true;
        
        return true;
      }

      // else, generate the PRUNE
      // If the PRUNE is UNSAT, the formula is UNSAT
      if(!semi_decision) {
        formula prune = encoder->prune(k);
        trace(trace_t::prune, prune);
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

  // function full of GCOV false negatives
  static check_result_t _check_syntax(
    term t, std::function<void(std::string)> const& err,
    std::vector<variable> const& scope,
    tsl::hopscotch_map<identifier, size_t> &rels,
    tsl::hopscotch_map<identifier, size_t> &funcs
  ) {
    return t.match( // LCOV_EXCL_LINE
      [](constant) -> check_result_t { return false; },
      [](variable) -> check_result_t { return false; },
      [&](application a) -> check_result_t {
        identifier id = a.func().name();
        size_t size = a.arguments().size();
        
        if(auto it = funcs.find(id); it != funcs.end() && it->second != size) {
          err(
            "Function '" + to_string(id) + 
            "' used twice with different arities"
          );
          return true;
        }

        if(rels.find(id) != rels.end()) {
          err(
            "Function symbol '" + to_string(id) + 
            "' already used as a relation symbol"
          );
          return true;
        }

        check_result_t res;
        for(term arg : a.arguments())
          res = res || _check_syntax(arg, err, scope, rels, funcs);

        if(!a.func().known_type())
          funcs.insert({id, size});
        return res;
      }, // LCOV_EXCL_LINE
      [&](constructor c) -> check_result_t {
        term arg = c.argument();
        if(!arg.is<variable>()) {
          err(
            "next()/wnext()/prev()/wprev() terms can only be applied "
            "directly to variables"
          );
          return true;
        }

        for(variable v : scope) { // LCOV_EXCL_LINE
          if(v == arg) {
            err(
              "next()/wnext()/prev()/wprev() terms cannot be applied "
              "to quantified variables"
            );
            return true;
          }
        }
        
        return {false, true, false};
      }
    );
  }

  // function full of GCOV false negatives
  static check_result_t _check_syntax(
    formula f, std::function<void(std::string)> const& err, 
    std::vector<variable> const& scope, bool positive, 
    tsl::hopscotch_map<identifier, size_t> &rels,
    tsl::hopscotch_map<identifier, size_t> &funcs
  ) {
    return f.match( // LCOV_EXCL_LINE
      [](boolean) -> check_result_t { return false; },
      [](proposition) -> check_result_t { return false; },
      [&](atom a) -> check_result_t {  
        identifier id = a.rel().name();
        size_t size = a.terms().size();
        if(auto it = rels.find(id); it != rels.end() && it->second != size) {
          err(
            "Relation '" + to_string(id) + 
            "' used twice with different arities"
          );
          return true;
        }

        if(funcs.find(id) != funcs.end()) {
          err(
            "Relation symbol '" + to_string(id) + 
            "' already used as a function symbol"
          );
          return true;
        }

        check_result_t res;
        for(term t : a.terms())
          res = res || _check_syntax(t, err, scope, rels, funcs);
        
        rels.insert({id, size});
        return res;  
      }, // LCOV_EXCL_LINE
      [&](quantifier q) -> check_result_t {
        std::vector<variable> s = scope;
        s.push_back(q.var());
        check_result_t res =
          _check_syntax(q.matrix(), err, s, positive, rels, funcs); // LCOV_EXCL_LINE
        if(res.has_next && res.has_disjunctions) { // LCOV_EXCL_LINE
          err( // LCOV_EXCL_LINE
            "next() terms and disjunctions cannot be mixed inside quantifiers"
          );
          return true; // LCOV_EXCL_LINE
        }

        return res;
      }, // LCOV_EXCL_LINE
      [&](negation, formula arg) {
        return _check_syntax(arg, err, scope, !positive, rels, funcs);
      },
      [&](big_disjunction o) {
        check_result_t r{false, false, positive};
        std::vector<check_result_t> results;
        for(auto op : o.operands()) {
          results.push_back(
            _check_syntax(op, err, scope, positive, rels, funcs)
          );
        }
        return std::accumulate(
          begin(results), end(results), r, std::logical_or<>{}
        );
      },
      [&](big_conjunction c) {
        check_result_t r{false, false, !positive};
        std::vector<check_result_t> results;
        for(auto op : c.operands()) {
          results.push_back(
            _check_syntax(op, err, scope, positive, rels, funcs)
          );
        }
        return std::accumulate(
          begin(results), end(results), r, std::logical_or<>{}
        );
      },
      [&](implication, formula left, formula right) {
        return _check_syntax(!left || right, err, scope, positive, rels, funcs);
      },
      [&](iff, formula left, formula right) {
        return _check_syntax( // LCOV_EXCL_LINE
          implies(left, right) && implies(right, left), 
          err, scope, positive, rels, funcs
        );
      },
      [&](tomorrow, formula arg) {
        return _check_syntax(arg, err, scope, positive, rels, funcs);
      },
      [&](w_tomorrow, formula arg) {
        return _check_syntax(arg, err, scope, positive, rels, funcs);
      },
      [&](yesterday, formula arg) {
        return _check_syntax(arg, err, scope, positive, rels, funcs);
      },
      [&](w_yesterday, formula arg) {
        return _check_syntax(arg, err, scope, positive, rels, funcs);
      },
      [&](temporal t) -> check_result_t {
        if(!scope.empty()) {
          err(
            "Temporal operators (excepting X/wX/Y/Z) cannot appear "
            "inside quantifiers"
          );
          return true;
        }

        return t.match( // LCOV_EXCL_LINE
          [&](unary, formula arg) {
            return _check_syntax(arg, err, scope, positive, rels, funcs);
          },
          [&](binary, formula left, formula right) {
            return _check_syntax(left, err, scope, positive, rels, funcs) || 
                   _check_syntax(right, err, scope, positive, rels, funcs);
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
      !_check_syntax(f, err, std::vector<variable>{}, true, rels, funcs).error;
  }

  

} // end namespace black::size_ternal
