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

#include <numeric>

namespace black::internal {
  formula sub_past(formula f) {
    alphabet *alpha = f.sigma();

    return f.match(
        [&](yesterday, formula op) {
          return alpha->prop(past_label{Y(sub_past(op))});
        },
        [&](w_yesterday, formula op) {
          return alpha->prop(past_label{Z(sub_past(op))});
        },
        [&](since, formula left, formula right) {
          return alpha->prop(past_label{S(sub_past(left), sub_past(right))});
        },
        [](triggered, formula left, formula right) {
          return sub_past(!S(!left, !right));
        },
        [](once p, formula op) { return sub_past(S(p.sigma()->top(), op)); },
        [](historically, formula op) { return sub_past(!O(!op)); },
        [](boolean b) { return b; },
        [](proposition p) { return p; },
        [](atom a) { return a; },
        [](unary u, formula op) {
          return unary(u.formula_type(), sub_past(op));
        },
        [](binary b, formula left, formula right) {
          return binary(b.formula_type(), sub_past(left), sub_past(right));
        }
    );
  }

  void gen_semantics(formula f, std::vector<formula> &sem) {
    return f.match(
        [](boolean) {},
        [](atom) {},
        [&](proposition a) {
          std::optional<past_label> label = a.label<past_label>();

          if (!label) return; // not a translator proposition

          formula psi = label->formula;
          return psi.match( // LCOV_EXCL_LINE
              [&](yesterday y, formula op) {
                formula sem_y = yesterday_semantics(a, y);

                sem.push_back(sem_y);

                gen_semantics(op, sem);
              },
              [&](w_yesterday z, formula op) {
                formula sem_z = w_yesterday_semantics(a, z);

                sem.push_back(sem_z);

                gen_semantics(op, sem);
              },
              [&](since s, formula left, formula right) {
                alphabet *alpha = f.sigma();
                proposition y = alpha->prop(past_label{Y(a)});
                formula sem_s = since_semantics(a, s, y);
                formula sem_y = yesterday_semantics(y, Y(a));

                sem.push_back(sem_s);
                sem.push_back(sem_y);

                gen_semantics(left, sem);
                gen_semantics(right, sem);
              },
              [](otherwise) { black_unreachable(); } // LCOV_EXCL_LINE
          );
        },
        [&](unary, formula op) { gen_semantics(op, sem); },
        [&](binary, formula left, formula right) {
          gen_semantics(left, sem);
          gen_semantics(right, sem);
        }
    );
  }

  formula remove_past(formula f) {
    formula ltl = sub_past(f);

    std::vector<formula> semantics;
    gen_semantics(ltl, semantics);

    // Conjoin the ltl formula with its semantics formulas
    return std::accumulate(semantics.begin(), semantics.end(), ltl,
        [](formula f1, formula f2) { return f1 && f2; }
    );
  }
} // namespace black::internal
