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

#include <ranges>
#include <vector>

namespace black::logic {

  //
  // boolean connectives
  //
  inline negation operator!(term_source auto t) {
    return negation(term{t});
  }
  
  inline conjunction operator&&(term_source auto t1, term_source auto t2) {
    return conjunction({term{t1}, term{t2}});
  }
  
  inline disjunction operator||(term_source auto t1, term_source auto t2) {
    return disjunction({term{t1}, term{t2}});
  }
  
  inline implication implies(term_source auto t1, term_source auto t2) {
    return implication(term{t1}, term{t2});
  }

  //
  // LTL operators
  //

  inline tomorrow X(term_source auto t) { return tomorrow(term{t}); }
  inline w_tomorrow wX(term_source auto t) { return w_tomorrow(term{t}); }
  inline eventually F(term_source auto t) { return eventually(term{t}); }
  inline always G(term_source auto t) { return always(term{t}); }
  inline until U(term_source auto t1, term_source auto t2) { 
    return until(term{t1}, term{t2}); 
  }
  inline release R(term_source auto t1, term_source auto t2) { 
    return release(term{t1}, term{t2}); 
  }
  inline yesterday Y(term_source auto t) { return yesterday(term{t}); }
  inline w_yesterday Z(term_source auto t) { return w_yesterday(term{t}); }
  inline once O(term_source auto t) { return once(term{t}); }
  inline historically H(term_source auto t) { return historically(term{t}); }
  inline since S(term_source auto t1, term_source auto t2) { 
    return since(term{t1}, term{t2}); 
  }
  inline triggered T(term_source auto t1, term_source auto t2) { 
    return triggered(term{t1}, term{t2}); 
  }

  //
  // Arithmetic operators
  //
  inline minus operator-(term_source auto t) {
    return minus(term{t});
  }
  
  inline sum operator+(term_source auto t1, term_source auto t2) {
    return sum(term{t1}, term{t2});
  }
  
  inline product operator*(term_source auto t1, term_source auto t2) {
    return product(term{t1}, term{t2});
  }

  inline difference operator-(term_source auto t1, term_source auto t2) {
    return difference(term{t1}, term{t2});
  }

  inline division operator/(term_source auto t1, term_source auto t2) {
    return division(term{t1}, term{t2});
  }

  //
  // Relational comparisons
  //
  inline less_than operator<(term_source auto t1, term_source auto t2) {
    return less_than(term{t1}, term{t2});
  }

  inline less_than_eq operator<=(term_source auto t1, term_source auto t2) {
    return less_than_eq(term{t1}, term{t2});
  }

  inline greater_than operator>(term_source auto t1, term_source auto t2) {
    return greater_than(term{t1}, term{t2});
  }

  inline greater_than_eq operator>=(term_source auto t1, term_source auto t2) {
    return greater_than_eq(term{t1}, term{t2});
  }

  //
  // Useful shortcuts
  //
  template<
    std::ranges::range T, typename V = std::ranges::range_value_t<T>,
    std::invocable<V> F
  >
    requires term_source<std::invoke_result_t<F, V>>
  inline conjunction big_and(T rng, F f = [](auto x) { return x; }) {
    std::vector<term> args;
    for(auto t : rng)
      args.push_back(term{f(t)});
    return conjunction(args);
  }
  
  template<
    std::ranges::range T, typename V = std::ranges::range_value_t<T>,
    std::invocable<V> F
  >
    requires term_source<std::invoke_result_t<F, V>>
  inline disjunction big_or(T rng, F f = [](auto x) { return x; }) {
    std::vector<term> args;
    for(auto t : rng)
      args.push_back(term{f(t)});
    return disjunction(args);
  }
}

#endif // BLACK_LOGIC_OPERATORS_HPP
