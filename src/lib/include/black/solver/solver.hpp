//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Luca Geatti
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

#ifndef BLACK_SOLVER_HPP
#define BLACK_SOLVER_HPP

#include <black/support/common.hpp>
#include <black/logic/logic.hpp>
#include <black/support/tribool.hpp>

#include <vector>
#include <variant>
#include <utility>
#include <limits>
#include <unordered_set>
#include <string>
#include <numeric>

namespace black_internal::solver {

  using namespace black::logic::fragments::LTLPFO;

  // main solver class
  class BLACK_EXPORT solver 
  {
    public:
      friend class model;

      // Checks if the formula `f` has a syntax supported by the solver
      //
      // Calls the callback function `err` with a description of the error
      static bool 
      check_syntax(formula f, std::function<void(std::string)> const& err);

      // Constructor and destructor
      solver();
      ~solver();

      solver(solver const&) = delete;
      solver &operator=(solver const&) = delete;
      solver(solver &&);
      solver &operator=(solver &&);

      // Solve the formula `f` over the scope `xi`, with up to `k_max'
      // iterations returning `tribool::undef` if `k_max` is reached
      //
      // If `semi_decision` is true, the termination rules for unsatisfiable
      // formulas are disabled, speeding up solving of satisfiable ones.
      //
      // If `finite` is `true` the formula is solved for the finite-trace
      // semantics.
      //
      // WARNING: `semi_decision = false` with first-order formulas using
      //          next(x) terms results in an *incomplete* algorithm.
      tribool solve(
        scope const& xi,
        formula f,
        bool finite = false,
        size_t k_max = std::numeric_limits<size_t>::max(),
        bool semi_decision = false
      );

      

      // Returns the model of the formula, if the last call to solve() 
      // returned true
      std::optional<class model> model() const;

      // Returns the last bound tried by the algorithm. The value returned 
      // does not make sense before the first call to solve()
      size_t last_bound() const;

      // Choose the SAT backend. The backend must exist.
      void set_sat_backend(std::string name);

      // Retrieve the current SAT backend
      std::string sat_backend() const;

      // Data type sent to the debug trace routine
      struct trace_t {
        enum type_t {
          stage,
          nnf,
          unravstd,
          empty,
          loop,
          prune
        };

        scope const *xi;
        type_t type;
        std::variant<
          size_t, 
          logic::formula<logic::LTLPFO>,
          logic::formula<logic::FO>
        > data;
      };

      // set the debug trace callback
      void set_tracer(std::function<void(trace_t)> const&tracer);

    private:
      struct _solver_t;
      std::unique_ptr<_solver_t> _data;

  }; // end class Black Solver

  class BLACK_EXPORT model
  {
    public:
      size_t size() const;
      size_t loop() const;
      tribool value(proposition a, size_t t) const;
      tribool value(atom a, size_t t) const;
    private:
      friend class solver;
      model(solver const&s) : _solver{s} { }
      
      solver const&_solver;
  };

} // end namespace black_internal

// Names exported to the user
namespace black {
  using black_internal::solver::solver;
}

#endif // SOLVER_HPP
