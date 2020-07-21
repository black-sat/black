//
// Created by gabriele on 16/07/2020.
//

#include <vector>
#include <black/logic/translator.hpp>

namespace black::internal {
  formula substitute_past(alphabet &alpha, formula f) {
    return f.match(
        [&](yesterday, formula op) {
          formula p = Y(substitute_past(alpha, op));
          std::string l = "Y";
          return alpha.var(std::pair(l, p));
        },
        [&](since, formula left, formula right) {
          formula p =
              S(substitute_past(alpha, left), substitute_past(alpha, right));
          std::string l = "S";
          return alpha.var(std::pair(l, p));
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
        [](otherwise) { black_unreachable(); });
  }

  std::vector<formula> gen_semantics(alphabet &alpha, formula f) {
    return f.match(
        [&](atom a) {
          std::vector<formula> semantics;
          std::optional<std::pair<std::string, formula>> label =
              a.label<std::pair<std::string, formula>>();

          if (label) {
            if ((*label).first == "Y" || (*label).first == "S") {
              formula psi = (*label).second;
              return psi.match(
                  [&](yesterday, formula op) {
                    formula s = !f && G(iff(X(f), op));

                    semantics = gen_semantics(alpha, op);
                    semantics.push_back(s);

                    return semantics;
                  },
                  [&](since, formula left, formula right) {
                    formula pl = Y(psi);
                    formula p = alpha.var(std::pair("Y", pl));

                    formula s = G(iff(f, right || (left && p)));

                    semantics = gen_semantics(alpha, p);

                    std::vector<formula> semanticsl =
                        gen_semantics(alpha, left);
                    std::vector<formula> semanticsr =
                        gen_semantics(alpha, right);
                    semantics.insert(semantics.end(), semanticsl.begin(),
                                     semanticsl.end());
                    semantics.insert(semantics.end(), semanticsr.begin(),
                                     semanticsr.end());

                    semantics.push_back(s);

                    return semantics;
                  },

                  // TODO: impossible cases but it doesn't compile without them
                  [&](boolean) { return semantics; },
                  [&](atom) { return semantics; },
                  [&](unary) { return semantics; },
                  [&](binary) { return semantics; },

                  [](otherwise) { black_unreachable(); });
            }
          }

          return semantics;
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
    if (fs.size() == 0) {
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
