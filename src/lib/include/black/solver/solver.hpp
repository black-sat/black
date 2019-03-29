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

namespace black::details {

  class solver {
    private:

      // Reference to the original alphabet
      alphabet& alpha;

      // Current LTL formula to solve
      formula frm;

      // Vector of all the X-requests of step k
      std::vector<tomorrow> xrequests;

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

      // Calls glucose to check if the boolean formula is sat
      bool is_sat(formula f);

      // Simple implementation of an allSAT solver
      formula all_sat(formula f);

    public:

      // Class constructor
      solver(alphabet &a)
        : alpha(a), frm(a.top()) { }

      // Class constructor
      solver(alphabet &a, formula f)
        : alpha(a), frm(f) { }

      // Conjoins the argument formula to the current one
      void add_formula(formula f) {
        if( frm == alpha.top() )
          frm = f;
        else
          frm = frm && f;
      }

      // Clears the input formula, setting it to True
      void clear() { frm = alpha.top(); }

      // Check for satisfiability of frm and
      // returns a model (if it is sat)
      formula solve(bool model_gen);

      bool bsc();

  }; // end class Black Solver

} // end namespace black::details

// Names exported to the user
namespace black {
  using details::solver;
}

#endif // SOLVER_HPP
