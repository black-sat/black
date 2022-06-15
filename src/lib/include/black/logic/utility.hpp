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

#ifndef BLACK_LOGIC_UTILITY_HPP_
#define BLACK_LOGIC_UTILITY_HPP_

#include <black/support/common.hpp>
#include <black/logic/logic.hpp>

namespace black::logic 
{
  //
  // This function tells whether a hierarchy object `h` contains any element
  // among those given as arguments. For example, if `f` is a formula,
  // `has_any_elements_of(f, syntax_element::boolean, syntax_element::iff)`
  // tells whether there is any boolean constant in the formula or any double
  // implication.
  //
  template<hierarchy H, typename ...Args>
    requires (std::is_same_v<Args, syntax_element> && ...)
  bool has_any_elements_of(H h, Args ...args) {
    if(((h.syntax_element() == args) || ...))
      return true;
    
    bool has = false;
    for_each_child(h, [&](auto child) {
      if(has_any_elements_of(child, args...))
        has = true;
    });

    return has;
  }

  //
  // The `remove_booleans` function remove boolean constants from a first-order
  // formula. This can only be done reliably for propositional and first-order
  // formulas because in temporal formulas the way to perform such removal would
  // depend on the finite/infinite-trace semantics. Implemented in logic.cpp.
  //
  BLACK_EXPORT
  formula<FO> remove_booleans(formula<FO> f);
}

#endif // BLACK_LOGIC_UTILITY_HPP_
