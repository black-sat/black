//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#ifndef BLACK_LOGIC_OPERATORS_HPP
#define BLACK_LOGIC_OPERATORS_HPP

namespace black::logic {


  //
  // boolean connectives
  //
  inline negation operator!(term t) {
    return negation(t);
  }
  
  inline conjunction operator&&(term t1, term t2) {
    return conjunction({t1, t2});
  }
  
  inline disjunction operator||(term t1, term t2) {
    return disjunction({t1, t2});
  }
  
  inline implication implies(term t1, term t2) {
    return implication(t1, t2);
  }

  //
  // LTL operators
  //

  inline tomorrow X(term t) { return tomorrow(t); }
  inline w_tomorrow wX(term t) { return w_tomorrow(t); }
  inline eventually F(term t) { return eventually(t); }
  inline always G(term t) { return always(t); }
  inline until U(term t1, term t2) { return until(t1, t2); }
  inline release R(term t1, term t2) { return release(t1, t2); }

  inline yesterday Y(term t) { return yesterday(t); }
  inline w_yesterday Z(term t) { return w_yesterday(t); }
  inline once O(term t) { return once(t); }
  inline historically H(term t) { return historically(t); }
  inline since S(term t1, term t2) { return since(t1, t2); }
  inline triggered T(term t1, term t2) { return triggered(t1, t2); }

  //
  // Arithmetic operators
  //
  inline minus operator-(term t) {
    return minus(t);
  }
  
  inline sum operator+(term t1, term t2) {
    return sum(t1, t2);
  }
  
  inline product operator*(term t1, term t2) {
    return product(t1, t2);
  }

  inline difference operator-(term t1, term t2) {
    return difference(t1, t2);
  }

  inline division operator/(term t1, term t2) {
    return division(t1, t2);
  }

  //
  // Relational comparisons
  //
  inline less_than operator<(term t1, term t2) {
    return less_than(t1, t2);
  }

  inline less_than_eq operator<=(term t1, term t2) {
    return less_than_eq(t1, t2);
  }

  inline greater_than operator>(term t1, term t2) {
    return greater_than(t1, t2);
  }

  inline greater_than_eq operator>=(term t1, term t2) {
    return greater_than_eq(t1, t2);
  }

}

#endif // BLACK_LOGIC_OPERATORS_HPP
