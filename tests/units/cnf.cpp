//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2020 Nicola Gigante
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

#include <catch2/catch.hpp>

#include <black/logic/formula.hpp>
#include <black/logic/parser.hpp>
#include <black/solver/solver.hpp>
#include <black/sat/cnf.hpp>

using namespace black;

TEST_CASE("CNF Translation")
{
  alphabet sigma;

  [[maybe_unused]] atom p = sigma.var("p");
  [[maybe_unused]] atom q = sigma.var("q");
  [[maybe_unused]] atom r = sigma.var("r");

  solver s{sigma};

  std::vector<formula> tests = {
    p && q, p || q, !r,
    p || (p && !q), (p && (!p || q)),
    implies(p, q), iff(p, q), sigma.top() && p,
    sigma.bottom()
  };

  for(formula f : tests) {
    DYNAMIC_SECTION("CNF translation for formula: " << f) {
      cnf c = to_cnf(f);
      formula fc = to_formula(sigma, c);
      s.assert_formula(!implies(fc,f));

      INFO("CNF: " << fc);
      REQUIRE(!s.solve());
    }
  }
}
