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

#ifndef BLACK_PAST_REMOVER_HPP
#define BLACK_PAST_REMOVER_HPP

#include <black/logic/alphabet.hpp>
#include <black/logic/parser.hpp>

#include <vector>

namespace black::internal {

  // Label data type for substituting past propositional letters
  struct past_label {
    class formula formula;

    bool operator==(const past_label &other) const {
      return formula == other.formula;
    }
  };

  // Substitute past operators with new propositional letters
  formula sub_past(formula);

  // Generate semantics for each new propositional letter (from substitute_past)
  void gen_semantics(formula, std::vector<formula>&);

  // Obtain semantics for yesterday propositional letter
  inline formula yesterday_semantics(proposition a, yesterday y) {
    return !a && G(iff(X(a), y.operand()));
  }

  // Obtain semantics for weak-yesterday propositional letter
  inline formula w_yesterday_semantics(proposition a, w_yesterday y) {
    return a && G(iff(X(a), y.operand()));
  }

  // Obtain semantics for since propositional letter
  inline formula since_semantics(proposition a, since s, proposition y) {
    return G(iff(a, s.right() || (s.left() && y)));
  }

  // Exposed procedure which puts together everything
  BLACK_EXPORT
  formula remove_past(formula);

} // end namespace black::internal

// Extend hash for past_label
namespace std
{
  template<>
  struct hash<black::internal::past_label> {
    size_t operator()(const black::internal::past_label &t) const {
      return
        hash<pair<black::internal::formula_type, black::internal::formula>>{}(
            pair(t.formula.formula_type(), t.formula)
        );
    }
  };
}

// Names exported to the user
namespace black {
  using internal::remove_past;
}

#endif //BLACK_PAST_REMOVER_HPP
