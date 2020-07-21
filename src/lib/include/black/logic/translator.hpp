//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Gabriele Venturato
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

#ifndef BLACK_TRANSLATOR_HPP
#define BLACK_TRANSLATOR_HPP

#include <vector>
#include <black/logic/alphabet.hpp>

namespace black::internal {

  // Substitute past operators with new propositional letters
  formula substitute_past(alphabet &, formula);

  // Generate semantics for each new propositional letter (from substitute_past)
  std::vector<formula> gen_semantics(alphabet &, formula);

  // Put all vector elements in conjunction
  formula conjoin_list(std::vector<formula> fs);

  // Exposed procedure which puts together everything
  formula ltlpast_to_ltl(alphabet &, formula);

} // end namespace black::internal

// Names exported to the user
namespace black {
  using internal::ltlpast_to_ltl;
}

#endif //BLACK_TRANSLATOR_HPP
