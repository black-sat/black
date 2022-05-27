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

using namespace black;

struct test {
  class formula formula;
  class formula result;
};

TEST_CASE("Translation idempotency on future only formulas")
{
  alphabet sigma;

  proposition p = sigma.prop("p");
  proposition q = sigma.prop("q");

  std::vector<formula> tests = {
      p, !p, X(p), F(p), G(p), XF(p), GF(p), XG(p),
      p && q, p || q, U(p,q), R(p,q),
      p && (X(U(p,q)) || XF(!q)),
      p && implies(X(R(p,q)), GF(!p)),
      U(p, !(GF(q))),
      !(iff(p || q, !q && p))
  };

  for(formula f : tests) {
    DYNAMIC_SECTION("Idempotency for formula: " << f) {
      CHECK(remove_past(f) == f);
    }
  }
}

TEST_CASE("Translation for basic past formulas")
{
  alphabet sigma;

  proposition p = sigma.prop("p");
  proposition q = sigma.prop("q");

  proposition p_Y  = internal::past_label(Y(p));
  proposition p_Z  = internal::past_label(Z(p));
  proposition p_S  = internal::past_label(S(p,q));
  proposition p_YS = internal::past_label(Y(p_S));
  proposition p_T  = internal::past_label(S(!p,!q));
  proposition p_YT = internal::past_label(Y(p_T));
  proposition p_P  = internal::past_label(S(sigma.top(),p));
  proposition p_YP = internal::past_label(Y(p_P));
  proposition p_H  = internal::past_label(S(sigma.top(),!p));
  proposition p_YH = internal::past_label(Y(p_H));

  std::vector<test> tests = {
      {Y(p),    p_Y && (!p_Y && G(iff(X(p_Y), p)))},
      {Z(p),    p_Z && (p_Z && G(iff(X(p_Z), p)))},
      {S(p,q),  (p_S && G(iff(p_S, q || (p && p_YS))))
                && (!p_YS && G(iff(X(p_YS), p_S)))},
      {T(p,q),  (!p_T && G(iff(p_T, !q || (!p && p_YT))))
                && (!p_YT && G(iff(X(p_YT), p_T)))},
      {O(p),    (p_P && G(iff(p_P, p || (sigma.top() && p_YP))))
                && (!p_YP && G(iff(X(p_YP), p_P)))},
      {H(p),    (!p_H && G(iff(p_H, !p || (sigma.top() && p_YH))))
                && (!p_YH && G(iff(X(p_YH), p_H)))}
  };

  for(test t : tests) {
    DYNAMIC_SECTION("Translation for formula: " << t.formula) {
      CHECK(remove_past(t.formula) == t.result);
    }
  }

  SECTION("Translation produces an equisatisfiable formula") {
    black::solver slv;

    for(test t : tests) {
      DYNAMIC_SECTION("Check for formula: " << t.formula) {
        slv.set_formula(
          formula{!implies(remove_past(t.formula), t.formula)}
        );
        CHECK(!slv.solve()); // check validity
      }
    }
  }
}
