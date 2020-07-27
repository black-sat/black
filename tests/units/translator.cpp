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
#include <black/logic/parser.hpp>

#include <catch2/catch.hpp>

using namespace black;

struct test {
  class formula formula;
  class formula result;
};

TEST_CASE("Translation idempotency on future only formulas")
{
  alphabet sigma;

  atom p = sigma.var("p");
  atom q = sigma.var("q");

  std::vector<formula> tests = {
      p, !p, X(p), F(p), G(p), XF(p), GF(p), XG(p),
      p && q, p || q, U(p,q), R(p,q),
      p && (X(U(p,q)) || XF(!q)),
      p && then(X(R(p,q)), GF(!p)),
      U(p, !(GF(q))),
      !(iff(p || q, !q && p))
  };

  for(formula f : tests) {
    DYNAMIC_SECTION("Idempotency for formula: " << f) {
      CHECK(ltlpast_to_ltl(sigma, f) == f);
    }
  }
}

TEST_CASE("Translation for basic past formulas")
{
  alphabet sigma;

  atom p = sigma.var("p");
  // atom q = sigma.var("q");

  formula p_Y = sigma.var(internal::past_label{Y(p)});

  std::vector<test> tests = { // , S(p,q), T(p,q), P(p), H(p)
      {Y(p), p_Y && (!p_Y && G(iff(X(p_Y), p)))}
  };

  for(test t : tests) {
    DYNAMIC_SECTION("Translation for formula: " << t.formula) {
      auto result = ltlpast_to_ltl(sigma, t.formula);

      CHECK(result == t.result);
    }
  }
}
