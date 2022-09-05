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
#include <black/solver/solver.hpp>

#include <catch.hpp>

#include <string_view>

using namespace black::logic::fragments::LTLP;
using black::remove_past;

struct test {
  formula f;
  formula result;
};

TEST_CASE("Translation idempotency on future only formulas")
{
  alphabet sigma;

  proposition p = sigma.proposition("p");
  proposition q = sigma.proposition("q");

  std::vector<formula> tests = {
      p, !p, X(p), F(p), G(p), X(F(p)), G(F(p)), X(G(p)),
      p && q, p || q, U(p,q), R(p,q),
      p && (X(U(p,q)) || X(F(!q))),
      p && implies(X(R(p,q)), G(F(!p))),
      U(p, !(G(F(q)))),
      !(iff(p || q, !q && p))
  };

  for(formula f : tests) {
    DYNAMIC_SECTION("Idempotency for formula: " << to_string(f)) {
      CHECK(black::remove_past(f) == f);
    }
  }
}

static
proposition past_label(formula f) {
  using namespace std::literals;
  return f.sigma()->proposition(std::tuple{"_past_label"sv, f});
}

TEST_CASE("Translation for basic past formulas")
{
  alphabet sigma;

  proposition p = sigma.proposition("p");
  proposition q = sigma.proposition("q");

  proposition p_Y  = past_label(Y(p));
  proposition p_Z  = past_label(Z(p));
  proposition p_S  = past_label(S(p,q));
  proposition p_YS = past_label(Y(p_S));
  proposition p_T  = past_label(S(!p,!q));
  proposition p_YT = past_label(Y(p_T));
  proposition p_P  = past_label(S(sigma.top(),p));
  proposition p_YP = past_label(Y(p_P));
  proposition p_H  = past_label(S(sigma.top(),!p));
  proposition p_YH = past_label(Y(p_H));

  std::vector<test> tests = {
      {Y(p),    p_Y && (!p_Y && G(implies(X(p_Y), p) && implies(p, wX(p_Y))))},
      {Z(p),    p_Z && (p_Z && G(implies(X(p_Z), p) && implies(p, wX(p_Z))))},
      {S(p,q),  (p_S && (G(iff(p_S, q || (p && p_YS)))
                && (!p_YS 
                  && G(implies(X(p_YS), p_S) && implies(p_S, wX(p_YS))))))},
      {T(p,q),  (!p_T && (G(iff(p_T, !q || (!p && p_YT)))
                && (!p_YT 
                  && G(implies(X(p_YT), p_T) && implies(p_T, wX(p_YT))))))},
      {O(p),    (p_P && (G(iff(p_P, p || (sigma.top() && p_YP)))
                && (!p_YP 
                  && G(implies(X(p_YP), p_P) && implies(p_P, wX(p_YP))))))},
      {H(p),    (!p_H && (G(iff(p_H, !p || (sigma.top() && p_YH)))
                && (!p_YH 
                  && G(implies(X(p_YH), p_H) && implies(p_H, wX(p_YH))))))}
  };

  for(test t : tests) {
    DYNAMIC_SECTION("Translation for formula: " << to_string(t.f)) {
      INFO("remove_past(f) = " << to_string(black::remove_past(t.f)));
      INFO("Translated f = " << to_string(t.result));
      
      CHECK(black::remove_past(t.f) == t.result);
    }
  }

  SECTION("Translation produces an equisatisfiable formula") {
    black::solver slv;

    for(test t : tests) {
      DYNAMIC_SECTION("Check for formula: " << to_string(t.f)) {
        slv.set_formula(
          formula{!implies(remove_past(t.f), t.f)}
        );
        CHECK(!slv.solve()); // check validity
      }
    }
  }
}
