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

#include <black/new/logic/logic.hpp>

namespace black::new_api::logic 
{
  template<hierarchy H>
  bool has_element(syntax_element e, H h) {
    if(h.syntax_element() == e)
      return true;
    
    bool has = false;
    for_each_child(h, [&](auto child) {
      if(has_element(e, h))
        has = true;
    });

    return has;
  }

  // formula simplify(formula f);

  // formula simplify_deep(formula f);

  // true if there is any true/false constant in the formula
}



#endif // BLACK_LOGIC_UTILITY_HPP_