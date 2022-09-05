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

#include <black/logic/past_remover.hpp>
#include <black/logic/prettyprint.hpp>

#include <numeric>

namespace black_internal::remove_past {

  using namespace black_internal::logic;

  // Label data type for substituting past propositional letters
  static
  proposition past_label(formula<LTLP> f) {
    using namespace std::literals;
    return f.sigma()->proposition(std::tuple{"_past_label"sv, f});
  }

  // Obtain semantics for yesterday propositional letter
  static
  formula<LTL> yesterday_semantics(proposition a, formula<LTL> f) {
    return !a && G(implies(X(a), f) && implies(f, wX(a)));
  }

  // Obtain semantics for weak-yesterday propositional letter
  static
  formula<LTL> w_yesterday_semantics(proposition a, formula<LTL> f) {
    return a && G(implies(X(a), f) && implies(f, wX(a)));
  }

  // Obtain semantics for since propositional letter
  static
  formula<LTL> since_semantics(
    proposition since, formula<LTL> l, formula<LTL> r, proposition y
  ) {
    return G(iff(since, r || (l && y))) && yesterday_semantics(y, since);
  }

  static
  formula<LTL> sub_past(formula<LTLP> f, std::vector<formula<LTL>> &sem) {
    return f.match( // LCOV_EXCL_LINE
        [&](boolean b) { return b; },
        [&](proposition p) { return p; },
        
        [&](yesterday<LTLP>, auto op) {
          auto sub = sub_past(op, sem);
          auto prop = past_label(Y(sub));
          sem.push_back(yesterday_semantics(prop, sub));

          return prop;
        },
        [&](w_yesterday<LTLP>, auto op) {
          auto sub = sub_past(op, sem);
          auto prop = past_label(Z(op));
          sem.push_back(w_yesterday_semantics(prop, sub));

          return prop;
        },
        [&](since<LTLP>, auto left, auto right) {
          auto lsub = sub_past(left, sem);
          auto rsub = sub_past(right, sem);
          auto prop = past_label(S(lsub, rsub));
          auto yprop = past_label(Y(prop));

          sem.push_back(since_semantics(prop, lsub, rsub, yprop));

          return prop;
        },
        [&](triggered<LTLP>, auto left, auto right) {
          return sub_past(!S(!left, !right), sem);
        },
        [&](once<LTLP> p, auto op) { 
          return sub_past(S(p.sigma()->top(), op), sem); 
        },
        [&](historically<LTLP>, auto op) { 
          return sub_past(!O(!op), sem); 
        },
        [&](unary<LTLP> u, auto arg) {
          return unary<LTL>(
            *fragment_cast<LTL>(u.node_type()), sub_past(arg, sem)
          );
        },
        [&](binary<LTLP> b, auto left, auto right) {
          return binary<LTL>(
            *fragment_cast<LTL>(b.node_type()),
            sub_past(left, sem), sub_past(right, sem)
          );
        }
    );
  }

  formula<LTL> remove_past(formula<LTLP> f) {
    
    std::vector<formula<LTL>> semantics;
    formula<LTL> ltl = sub_past(f, semantics);

    // Conjoin the ltl formula with its semantics formulas
    return // LCOV_EXCL_LINE
      std::accumulate(semantics.begin(), semantics.end(), ltl, // LCOV_EXCL_LINE
        [](auto f1, auto f2) { return f1 && f2; }
    );
  }
} // namespace black_internal
