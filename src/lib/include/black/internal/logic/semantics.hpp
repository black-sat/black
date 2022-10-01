//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#ifndef BLACK_LOGIC_SEMANTICS_HPP
#define BLACK_LOGIC_SEMANTICS_HPP

//
// This file contains code that deals with the semantic aspects of formulas and
// terms. These include type checking and other semantic checks to be done on
// formulas to ensure their correctness before being given to solvers or
// printers.
//
namespace black_internal::logic {
  
  //
  // This function recursively computes the sort of a term. 
  // The result is meaningless if the term is not well-typed.
  //
  template<is_term T>
  sort sort_of(T t) {
    using Syntax = typename T::syntax;
    alphabet &sigma = *t.sigma();
    return t.match(
      [&](constant<Syntax>, auto value) {
        return value.match(
          [&](integer) { return sigma.integer_sort(); },
          [&](real)    { return sigma.real_sort(); }
        );
      },
      [&](variable v) {
        return v.sort();
      },
      [&](application<Syntax> app) {
        return app.func().result();
      },
      [&](to_integer<Syntax>) {
        return t.sigma()->integer_sort();
      },
      [&](to_real<Syntax>) {
        return t.sigma()->real_sort();
      },
      [&](unary_term<Syntax>, auto arg) { 
        return sort_of(arg);
      },
      [&](division<Syntax>) {
        return sigma.real_sort();
      },
      [&](int_division<Syntax>) {
        return sigma.integer_sort();
      },
      [&](binary_term<Syntax>, auto left, auto right) -> sort {
        if(sort_of(left) == sigma.integer_sort() &&
           sort_of(right) == sigma.integer_sort())
          return sigma.integer_sort();
        
        return sigma.real_sort();
      }
    );
  }
}

#endif
