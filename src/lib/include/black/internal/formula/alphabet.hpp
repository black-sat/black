//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#ifndef BLACK_ALPHABET_IMPL_HPP
#define BLACK_ALPHABET_IMPL_HPP

#include <black/support/meta.hpp>
#include <black/logic/formula.hpp>

namespace black::internal {

  //
  // Out-of-line definitions of methods of class `alphabet`
  //
  template<typename T, REQUIRES_OUT_OF_LINE(internal::is_hashable<T>)>
  inline proposition alphabet::prop(T&& label) {
    if constexpr(std::is_constructible_v<std::string,T>) {
      return
        proposition{this, allocate_proposition(std::string{FWD(label)})};
    } else {
      return proposition{this, allocate_proposition(FWD(label))};
    }
  }

  // Out-of-line implementations from the handle_base class in formula.hpp,
  // placed here to have a complete alphabet type
  template<typename H, typename F>
  template<typename FType, typename Arg>
  std::pair<alphabet *, unary_t *>
  handle_base<H, F>::allocate_unary(FType type, Arg const&arg)
  {
    // The type is templated only because of circularity problems
    static_assert(std::is_same_v<FType, unary::type>);

    // Get the alphabet from the argument
    class alphabet *sigma = arg._alphabet;

    // Ask the alphabet to actually allocate the formula
    unary_t *object = sigma->allocate_unary(type, arg._formula);

    return {sigma, object};
  }

  template<typename H, typename F>
  template<typename FType, typename Arg1, typename Arg2>
  std::pair<alphabet *, binary_t *>
  handle_base<H, F>::allocate_binary(FType type,
                                     Arg1 const&arg1, Arg2 const&arg2)
  {
    // The type is templated only because of circularity problems
    static_assert(std::is_same_v<FType, binary::type>);

    // Check that both arguments come from the same alphabet
    black_assert(arg1._alphabet == arg2._alphabet);

    // Get the alphabet from the first argument (same as the second, by now)
    class alphabet *sigma = arg1._alphabet;

    // Ask the alphabet to actually allocate the formula
    binary_t *object = sigma->allocate_binary(
      type, arg1._formula, arg2._formula
    );

    return {sigma, object};
  }
  
} // namespace black::internal

/*
 * Functions from formula.hpp that need the alphabet class
 */
namespace black::internal {
   
  // Conjunct multiple formulas generated from a range,
  // avoiding useless true formulas at the beginning of the fold
  template<typename Iterator, typename EndIterator, typename F>
  formula big_and(alphabet &sigma, Iterator b, EndIterator e, F&& f) {
    formula acc = sigma.top();

    while(b != e) {
      formula elem = std::forward<F>(f)(*b++);
      if(elem == sigma.top())
        continue;
      else if(acc == sigma.top())
        acc = elem;
      else
        acc = acc && elem;
    }

    return acc;
  }

  template<typename Range, typename F>
  formula big_and(alphabet &sigma, Range r, F&& f) {
    return big_and(sigma, begin(r), end(r), std::forward<F>(f));
  }
   
  // Disjunct multiple formulas generated from a range,
  // avoiding useless true formulas at the beginning of the fold
  template<typename Iterator, typename EndIterator, typename F>
  formula big_or(alphabet &sigma, Iterator b, EndIterator e, F&& f) 
  {
    formula acc = sigma.bottom();

    while(b != e) {
      formula elem = std::forward<F>(f)(*b++);
      if(elem == sigma.bottom())
        continue;
      else if(acc == sigma.bottom())
        acc = elem;
      else
        acc = acc || elem;
    }

    return acc;
  }

  template<typename Range, typename F>
  formula big_or(alphabet &sigma, Range r, F&& f) {
    return big_or(sigma, begin(r), end(r), std::forward<F>(f));
  }

}

#endif // BLACK_ALPHABET_IMPL_HPP
