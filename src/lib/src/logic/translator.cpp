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

#include <black/logic/translator.hpp>

namespace black::internal {
  formula substitute_past(alphabet &alpha, formula f) {
    return f.match(
        [&](yesterday, formula op) {
          formula p = Y(substitute_past(alpha, op));
          return alpha.var(past_label{p});
        },
        [&](since, formula left, formula right) {
          formula p =
              S(substitute_past(alpha, left), substitute_past(alpha, right));
          std::string l = "S";
          return alpha.var(past_label{p});
        },
        [&](triggered, formula left, formula right) {
          return substitute_past(alpha, !S(!left, !right));
        },
        [&](past, formula op) {
          return substitute_past(alpha, S(alpha.top(), op));
        },
        [&](historically, formula op) {
          return substitute_past(alpha, !P(!op));
        },
        [](boolean b) { return b; },
        [](atom a) { return a; },
        [&](unary u, formula op) {
          return unary(u.formula_type(), substitute_past(alpha, op));
        },
        [&](binary b, formula left, formula right) {
          return binary(b.formula_type(), substitute_past(alpha, left),
                        substitute_past(alpha, right));
        },
        [](otherwise) { black_unreachable(); }
    );
  }

  std::vector<formula> gen_semantics(alphabet &alpha, formula f) {
    return f.match(
        [&](atom a) {
          std::vector<formula> semantics;
          std::optional<past_label> label = a.label<past_label>();

          if (!label) return semantics;

          formula psi = label->formula;
          return psi.match(
              [&](yesterday y, formula op) {
                formula sem = yesterday_semantics(a, y);

                semantics = gen_semantics(alpha, op);
                semantics.push_back(sem);

                return semantics;
              },
              [&](since s, formula left, formula right) {
                atom y = alpha.var(past_label{Y(a)});

                formula sem = since_semantics(a, s, y);
                formula semy = yesterday_semantics(y, Y(a));

                std::vector<formula> semanticsl =
                    gen_semantics(alpha, left);
                std::vector<formula> semanticsr =
                    gen_semantics(alpha, right);
                semantics.insert(semantics.end(), semanticsl.begin(),
                                 semanticsl.end());
                semantics.insert(semantics.end(), semanticsr.begin(),
                                 semanticsr.end());

                semantics.push_back(sem);
                semantics.push_back(semy);

                return semantics;
              },
              [](otherwise) -> std::vector<formula> { black_unreachable(); }
          );
        },
        [](boolean) {
          std::vector<formula> semantics;
          return semantics;
        },
        [&](unary, formula op) {
          std::vector<formula> semantics = gen_semantics(alpha, op);
          return semantics;
        },
        [&](binary, formula left, formula right) {
          std::vector<formula> semanticsl = gen_semantics(alpha, left);
          std::vector<formula> semanticsr = gen_semantics(alpha, right);
          semanticsl.insert(semanticsl.end(), semanticsr.begin(),
                            semanticsr.end());
          return semanticsl;
        },
        [](otherwise) { black_unreachable(); });
  }

  formula conjoin_list(std::vector<formula> fs) {
    if (fs.empty()) {
      black_unreachable();
    } else if (fs.size() == 1) {
      formula f = fs.back();
      fs.pop_back();
      return f;
    } else {
      formula f = fs.back();
      fs.pop_back();
      return f && conjoin_list(fs);
    }
  }

  formula ltlpast_to_ltl(alphabet &alpha, formula f) {
    formula ltl = substitute_past(alpha, f);
    std::vector<formula> semantics = gen_semantics(alpha, ltl);
    semantics.push_back(ltl);

    return conjoin_list(semantics);
  }
} // namespace black::internal
