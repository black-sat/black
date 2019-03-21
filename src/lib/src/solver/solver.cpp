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

#include <black/solver/solver.hpp>

namespace black::details {
  
  // Class constructor
  //solver::solver() : frm(top()) {}

  // Class constructor
  solver::solver(formula f) : frm(f) {}

  // Conjoins the argument formula to the current one
  void solver::add_formula(formula f) {
    frm = frm && f;
  }

  // Check for satisfiability of frm and a model (if it is sat)
  bool solver::solve(bool = false) {
    [[maybe_unused]]
    formula xnf_frm = to_xnf(frm);

    return false;
  }

  // Turns the current formula into Next Normal Form
  formula to_xnf(formula f) {
    return f.match(
      // Future Operators
      [&](boolean)      { return f; },
      [&](atom)         { return f; },
      [&](tomorrow)     { return f; },
      [&](negation)     { return formula{!to_xnf(f)}; },
      [](conjunction c) { return formula{to_xnf(c.left()) && to_xnf(c.right())}; }, 
      [](disjunction d) { return formula{to_xnf(d.left()) || to_xnf(d.right())}; },
      [](until u)       { 
                          return formula{to_xnf(u.right()) || (to_xnf(u.left()) && X(u))};
                        },
      [](eventually e)  { return formula{to_xnf(e.operand()) || X(e)}; },
      [](always a)      { return formula{to_xnf(a.operand()) && X(a)}; },
      [](release r)     { 
                          return formula{to_xnf(r.right()) && (to_xnf(r.left()) || X(r))};
                        },
      // TODO: past operators
      [&](otherwise) { black_unreachable(); return f; }
    );
  }


} // end namespace black::details


