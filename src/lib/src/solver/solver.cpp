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
  solver::solver(alphabet &a, formula f) : 
    alpha(a),
    frm(f)
  {}

  // Conjoins the argument formula to the current one
  void solver::add_formula(formula f) {
    frm = frm && f;
  }

  // Checks for satisfiability of frm and 
  // returns a model (if it is sat)
  bool solver::solve(bool = false) {
    int k=0;
    formula encoding = alpha.boolean(true);
    while(true){
      // Generating the k-unraveling
      encoding = encoding && k_unraveling(k);
      if(!is_sat(encoding)){
        return false;
      }
      k++;
    } // end while(true)
  }
  


  // Generates the k-unraveling for the given k.
  formula solver::k_unraveling(int k) {
    // Copy of the X-requests generated in phase k-1.
    std::vector<unary> current_xreq = xrequests;
    if(k==0){
      return to_ground_xnf(frm,k);
    }else{
      xrequests.clear(); // clears all the X-requests from the vector
      formula big_and = alpha.boolean(true);
      for(auto it = current_xreq.begin(); it != current_xreq.end(); it++){
        // X(alpha)_P^{k-1}
        formula left_hand = alpha.var(std::pair<formula,int>(formula{*it},k-1));
        // xnf(\alpha)_P^{k}
        formula right_hand = to_ground_xnf((*it).operand(),k); 
        // left_hand IFF right_hand
        big_and = big_and && iff(left_hand, right_hand);
      }
      return big_and;  
    }
  }


  // Turns the current formula into Next Normal Form
  formula solver::to_ground_xnf(formula f, int k) {
    return f.match(
      // Future Operators
      [&](boolean)      { return f; },
      [&](atom)         { return f; },
      [&,k](tomorrow t)   { xrequests.push_back(t);
                          return formula{alpha.var(std::pair<formula,int>(formula{t},k))}; },
      [this,k](negation n)    { return formula{!to_ground_xnf(n.operand(),k)}; },
      [this,k](conjunction c) { return formula{to_ground_xnf(c.left(),k) && to_ground_xnf(c.right(),k)}; },
      [this,k](disjunction d) { return formula{to_ground_xnf(d.left(),k) || to_ground_xnf(d.right(),k)}; },
      [this,k](until u) { xrequests.push_back(X(u));
                        return formula{to_ground_xnf(u.right(),k) || 
                                 (to_ground_xnf(u.left(),k) && alpha.var(std::pair<formula,int>(formula{X(u)},k)) 
                      )};},
      [this,k](eventually e) { xrequests.push_back(X(e));
                             return formula{to_ground_xnf(e.operand(),k) || 
                             alpha.var(std::pair<formula,int>(formula{X(e)},k))};
                           },
      [this,k](always a)  { xrequests.push_back(X(a));
                          return formula{to_ground_xnf(a.operand(),k) 
                          && alpha.var(std::pair<formula,int>(formula{X(a)},k))};
                        },
      [this,k](release r) { xrequests.push_back(X(r));
                          return formula{to_ground_xnf(r.right(),k) && 
                                 (to_ground_xnf(r.left(),k) || 
                                 alpha.var(std::pair<formula,int>(formula{X(r)},k)))};
                        },
      // TODO: past operators
      [&](otherwise) { black_unreachable(); return f; }
    );
  }



  bool solver::is_sat(formula /*encoding*/) {
    // Transformation to CNF
    // ...
    // Call to Glucose
    // ...
    bool res = true;
    return res;
  }




} // end namespace black::details
