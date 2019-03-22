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
      std::vector<unary> xrequests; 
      
      // Vector of all the X-eventualities of step k
      std::vector<unary> xeventualities;

      // Generates the k-unraveling for the given k
      formula k_unraveling(int k); 
      
      // Generates the Next Normal Form of f
      formula to_ground_xnf(formula f, int k);
    
    public:
      
      // Class constructor
      //solver();
      
      // Class constructor
      solver(alphabet &a, formula f);
      
      // Conjoins the argument formula to the current one
      void add_formula(formula f); 
      
      // Check for satisfiability of frm and 
      // returns a model (if it is sat)
      bool solve(bool model_gen);
           
  }; // end class Black Solver

} // end namespace black::details

// Names exported to the user
namespace black {
  using details::solver;
}

#endif // SOLVER_HPP
