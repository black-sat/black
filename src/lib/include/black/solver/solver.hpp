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

#include <black/logic/formula.hpp>
#include <black/logic/alphabet.hpp>
#include <black/sat/mathsat.hpp>

#include <vector>
#include <utility>
#include <limits>
#include <unordered_set>

namespace black::internal {

  // Auxiliary functions

  // Transformation in NNF
  formula to_nnf(formula);

  // main solver class
  class solver 
  {
    public:

      // Class constructor
      explicit solver(alphabet &a)
        : _alpha(a), _frm(a.top()) { }

      // Class constructor
      solver(alphabet &a, formula f)
        : _alpha(a), _frm(to_nnf(f)) { }

      // Asserts a formula
      void assert_formula(formula f);

      // Clears the solver set of formulas
      void clear();

      // Solve the formula with up to `k_max' iterations
      bool solve(std::optional<int> k_max = std::nullopt);

    private:

      // Extract the x-eventuality from an x-request
      std::optional<formula> get_xev(tomorrow xreq);

      // Generates the PRUNE encoding
      formula prune(int k);

      // Generates the _lPRUNE_j^k encoding
      formula l_j_k_prune(int l, int j, int k);

      // Generates the EMPTY and LOOP encoding
      formula empty_and_loop(int k);

      // Generates the encoding for EMPTY_k
      formula k_empty(int k);

      // Generates the encoding for LOOP_k
      formula k_loop(int k);

      // Generates the encoding for _lP_k
      formula l_to_k_period(int l, int k);

      // Generates the encoding for _lL_k
      formula l_to_k_loop(int l, int k);

      // Generates the k-unraveling for the given k
      formula k_unraveling(int k);

      // Generates the Next Normal Form of f
      formula to_ground_xnf(formula f, int k, bool update);

      // X-requests from the formula's closure
      void add_xclosure(formula f);
  
    private:
      // Reference to the original _alphabet
      alphabet& _alpha;

      // Current LTL formula to solve
      formula _frm;

      // Vector of all the X-requests of step k
      std::vector<tomorrow> _xrequests;

      // X-requests from the closure of the formula
      // TODO: specialize to std::unordered_set<tomorrow>
      std::vector<tomorrow> _xclosure;


  }; // end class Black Solver

  // simple public functions are given an inlinable implementation below
  inline void solver::assert_formula(formula f) {
    f = to_nnf(f);
    add_xclosure(f);
    if( _frm == _alpha.top() )
      _frm = f;
    else
      _frm = _frm && f;
  }

  inline void solver::clear() {
    _frm = _alpha.top();
    _xrequests.clear();
    _xclosure.clear();
  }

} // end namespace black::internal

// Names exported to the user
namespace black {
  using internal::solver;
  using internal::to_nnf;
}

#endif // SOLVER_HPP
