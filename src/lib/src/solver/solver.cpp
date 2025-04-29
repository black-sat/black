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
#include <atomic>
#include <memory>
#include <future>

namespace black_internal::solver
{
  /*
   * Private implementation of the solver class.
   */
  struct solver::_solver_t : std::enable_shared_from_this<solver::_solver_t>
  {
    // whether a model has been found 
    // i.e., whether solve() has been called and returned true
    bool model = false;

    // encoder object used in the last solve() call
    std::optional<encoder::encoder> enc;

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

    // the flag for `interrupt()`
    std::atomic<bool> interrupt_flag = false;

    // tracer
    std::function<void(trace_t)> tracer = [](trace_t){};

    void trace(size_t k);
    void trace(trace_t::type_t, scope const&, logic::formula);

    // Main algorithm
    tribool solve(
      scope const& xi, logic::formula f, 
      bool finite, size_t k_max, std::optional<std::chrono::seconds> timeout, 
      bool semi_decision
    );

    void interrupt();
  };

  solver::solver() : _data{std::make_unique<_solver_t>()} { }
  solver::~solver() = default;

  solver::solver(solver &&) = default;
  solver &solver::operator=(solver &&) = default;

  tribool solver::solve(
    scope const& xi, logic::formula f, 
    bool finite, size_t k_max, std::optional<std::chrono::seconds> timeout,
    bool semi_decision
  ) {
    return _data->solve(xi, f, finite, k_max, timeout, semi_decision);
  }

  
  tribool solver::is_valid(
    scope const& xi, logic::formula f, 
    bool finite, size_t k_max, std::optional<std::chrono::seconds> timeout,
    bool semi_decision
  ) {
    tribool res = _data->solve(xi, !f, finite, k_max, timeout, semi_decision);
    if(res == true)
      return false;
    if(res == false)
      return true;
    return tribool::undef;
  }

  void solver::interrupt() {
    _data->interrupt();
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
    using black_internal::encoder::sp_witness_t;
    using black_internal::encoder::encoder;
    black_assert(size() > 0);
    
    size_t k = size() - 1;
    for(size_t l = 0; l < k; ++l) {
      sp_witness_t starws = { 
        _solver._data->sigma->star(), std::nullopt 
      };
      proposition loop_prop = 
        encoder::loop_prop(_solver._data->sigma, l, k, starws, starws);
      tribool value = _solver._data->sat->value(loop_prop);
      
      if(value == true)
        return l + 1;
    }

    return size();
  }

  tribool model::value(proposition a, size_t t) const {
    using black_internal::encoder::encoder;
    proposition u = encoder::stepped(a, t);

    return _solver._data->sat->value(u);
  }

  tribool model::value(atom a, size_t t) const {
    if(!_solver._data->enc.has_value())
      return tribool::undef;

    logic::atom u = _solver._data->enc->stepped(a, t);

    return _solver._data->sat->value(u);
  }

  tribool model::value(equality e, size_t t) const {
    if(!_solver._data->enc.has_value())
      return tribool::undef;

    logic::equality u = _solver._data->enc->stepped(e, t);

    return _solver._data->sat->value(u);
  }

  tribool model::value(comparison c, size_t t) const {
    if(!_solver._data->enc.has_value())
      return tribool::undef;

    logic::comparison u = _solver._data->enc->stepped(c, t);

    return _solver._data->sat->value(u);
  }

  void solver::_solver_t::trace(size_t k){
    tracer({nullptr, trace_t::stage, {k}});
  }

  void solver::_solver_t::trace(
    trace_t::type_t type, scope const& xi, logic::formula f
  ){
    tracer({&xi, type, {f}});
  }

  /*
   * Main algorithm. Solve the formula with up to `k_max' iterations.
   * If semi_decision = true, we disable the PRUNE rule.
   */
  tribool solver::_solver_t::solve(
    scope const& s, logic::formula f, bool finite, 
    size_t k_max, std::optional<std::chrono::seconds> timeout, 
    bool semi_decision
  ) {
    scope xi = chain(s);
    
    enc = encoder::encoder{f, xi, finite};
    sat = black::sat::solver::get_solver(sat_backend, xi);

    trace(trace_t::nnf, xi, enc->get_formula());

    sigma = f.sigma();
    model = false;
    model_size = 0;
    last_bound = 0;

    if(timeout)
      std::thread([timeout, self = this->shared_from_this()]() {
        std::this_thread::sleep_for(*timeout);
        self->interrupt();
      }).detach();

    for(size_t k = 0; !interrupt_flag && k <= k_max; last_bound = k++)
    {
      trace(k);
      // Generating the k-unraveling.
      // If it is UNSAT, then stop with UNSAT
      auto unrav = enc->k_unraveling(k);
      trace(trace_t::unrav, xi, unrav);
      sat->assert_formula(unrav);
      if(tribool res = sat->is_sat(); !res)
        return res;

      // else, continue to check EMPTY and LOOP.
      // If the k-unrav is SAT assuming EMPTY or LOOP, then stop with SAT
      auto empty = enc->k_empty(k);
      auto loop = enc->k_loop(k);
      trace(trace_t::empty, xi, empty);
      if(sat->is_sat_with(empty)) {
        model_size = k + 1;
        model = true;
        
        return true;
      }
      trace(trace_t::loop, xi, loop);
      if(sat->is_sat_with(loop)) {
        model_size = k + 1;
        model = true;
        
        return true;
      }

      // else, generate the PRUNE
      // If the PRUNE is UNSAT, the formula is UNSAT
      if(!semi_decision) {
        auto prune = enc->prune(k);
        trace(trace_t::prune, xi, prune);
        sat->assert_formula(!prune);
        if(tribool res = sat->is_sat(); !res)
          return res;
      }
    } // end for
    interrupt_flag = false;

    return tribool::undef;
  }

  void solver::_solver_t::interrupt() {
    interrupt_flag = true;
    sat->interrupt();
  }

  template<hierarchy H, typename F>
  void _check_syntax(H h, bool quantified, F err) {
    if(h.template is<quantifier>())
      quantified = true;

    for_each_child(h, [&](auto child){
      child.match(
        [&](unary_term t, term arg){
          t.match(
            [](negative) { },
            [](to_real) { },
            [](to_integer) { },
            [&](otherwise) {
              if(!arg.is<variable>())
                err(
                  "next()/wnext()/prev()/wprev() terms can only be applied "
                  "directly to variables"
                );
            }
          );
        },
        [](tomorrow) { },
        [](w_tomorrow) { },
        [](yesterday) { },
        [](w_yesterday) { },
        [&](otherwise) { 
          if(!quantified)
            return;
          
          bool temporal = child.match(
            [](always) { return true; },
            [](eventually) { return true; },
            [](once) { return true; },
            [](historically) { return true; },
            [](until) { return true; },
            [](release) { return true; },
            [](w_until) { return true; },
            [](s_release) { return true; },
            [](since) { return true; },
            [](triggered) { return true; },
            [](otherwise) { return false; }
          );
          if(temporal)
            err(
              "Temporal operators (excepting X/wX/Y/Z) cannot appear "
              "inside quantifiers"
            );
        }
      );
      _check_syntax(child, quantified, err);
    });
  }

  bool 
  solver::check_syntax(formula f, std::function<void(std::string)> const&err) 
  {
    bool ok = true;

    _check_syntax(f, false, [&](auto msg) {
      ok = false;
      err(msg);
    });

    return ok;
  }

  

} // end namespace black::internal
