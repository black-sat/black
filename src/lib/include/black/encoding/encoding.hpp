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

#ifndef BLACK_ENCODING_HPP
#define BLACK_ENCODING_HPP

#include <black/logic/formula.hpp>

namespace black::details {
  
  class BlackSolver {
    private:
      // Current LTL formula to solve
      formula frm;
      formula to_xnf(formula f);
    public:
      // Class constructor
      BlackSolver();
      // Class constructor
      BlackSolver(formula f);
      // Conjoin the argument formula to the current one
      void add_formula(formula f); 
      // Check for satisfiability of frm and a model (it is sat)
      bool solve(bool model_gen);

           
  }; // end class Black Solver

} // end namespace black::details

// Names exported to the user
namespace black {
  //using details::solve;
}

#endif // BLACK_ENCODING_HPP
