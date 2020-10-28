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

#include <vector>
#include <utility>
#include <limits>
#include <unordered_set>
#include <string>
#include <numeric>

#include <tsl/hopscotch_map.h>

namespace black::internal {

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

      // Choose the SAT backend. The backend must exist.
      void set_sat_backend(std::string name);

      // Retrieve the current SAT backend
      std::string sat_backend() const;

    private:

      // Extract the x-eventuality from an x-request
      static std::optional<formula> get_xev(tomorrow xreq);

      // Generates the PRUNE encoding
      formula prune(int k);

      // Generates the _lPRUNE_j^k encoding
      formula l_j_k_prune(int l, int j, int k);

      // Generates the EMPTY and LOOP encoding
      formula empty_or_loop(int k);

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

      // Generates the Stepped Normal Form of f
      formula to_ground_snf(formula f, int k);

      formula to_nnf(formula f);
      formula to_nnf_inner(formula f);

      // collect X/Y/Z-requests
      void add_xyz_requests(formula f);
  
    private:
      // Reference to the original _alphabet
      alphabet& _alpha;

      // Current LTL formula to solve
      formula _frm;

      // X/Y/Z-requests from the formula's closure
      // TODO: specialize to std::unordered_set<tomorrow/yesterday/w_yesterday>
      std::vector<tomorrow> _xrequests;
      std::vector<yesterday> _yrequests;
      std::vector<w_yesterday> _zrequests;

      // cache to memoize to_nnf() calls
      tsl::hopscotch_map<formula, formula> _nnf_cache;

      // the name of the currently chosen sat backend
      std::string _sat_backend = "z3"; // sensible default
  }; // end class Black Solver

  // simple public functions are given an inlineable implementation below
  inline void solver::assert_formula(formula f) {
    f = to_nnf(f);
    add_xyz_requests(f);
    if( _frm == _alpha.top() )
      _frm = f;
    else
      _frm = _frm && f;
  }

  inline void solver::clear() {
    _frm = _alpha.top();
    _xrequests.clear();
    _yrequests.clear();
    _zrequests.clear();
  }

} // end namespace black::internal

// Names exported to the user
namespace black {
  using internal::solver;
}

#endif // SOLVER_HPP
