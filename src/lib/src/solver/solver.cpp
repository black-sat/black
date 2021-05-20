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

#include <black/solver/solver.hpp>
#include <black/solver/encoding.hpp>
#include <black/sat/solver.hpp>
#include <black/support/range.hpp>

#include <fmt/format.h>

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
    std::string sat_backend = "z3"; // sensible default

    // Main algorithm
    tribool solve(size_t k_max);
  };

  solver::solver() : _data{std::make_unique<_solver_t>()} { }
  solver::~solver() = default;

  void solver::set_formula(formula f) {
    _data->encoder = encoder{f};
  }

  tribool solver::solve(size_t k_max) {
    return _data->solve(k_max);
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
      atom loop_var = 
        _solver._data->encoder->sigma->var(std::tuple{"_loop_var", l, k});
      tribool value = _solver._data->sat->value(loop_var);
      if(value == true || value == tribool::undef)
        return l + 1;
    }

    return size();
  }

  tribool model::value(atom a, size_t t) const {
    black_assert(_solver._data->encoder);
    
    atom u = _solver._data->encoder->ground(a, t);

    return _solver._data->sat->value(u);
  }

  /*
   * Main algorithm. Solve the formula with up to `k_max' iterations
   */
  tribool solver::_solver_t::solve(size_t k_max)
  {
    if(!encoder)
      return tribool::undef;
    
    sat = sat::solver::get_solver(sat_backend);

    last_bound = 0;
    for(size_t k = 0; k <= k_max; last_bound = k++)
    {
      // Generating the k-unraveling.
      // If it is UNSAT, then stop with UNSAT
      sat->assert_formula(encoder->k_unraveling(k));
      if(!sat->is_sat())
        return model = false;

      // else, continue to check EMPTY and LOOP.
      // If the k-unrav is SAT assuming EMPTY or LOOP, then stop with SAT
      if(sat->is_sat_with(encoder->k_empty(k) || encoder->k_loop(k))) {
        model_size = k + 1;
        model = true;
        
        return true;
      }

      // else, generate the PRUNE
      // If the PRUNE is UNSAT, the formula is UNSAT
      sat->assert_formula(!encoder->prune(k));
      if(!sat->is_sat())
        return model = false;
    } // end for

    model = false;
    return tribool::undef;
  }

} // end namespace black::size_ternal
