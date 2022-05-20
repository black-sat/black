//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2021 Nicola Gigante
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

#include <black/logic/alphabet.hpp>
#include <black/solver/solver.hpp>
#include <black/logic/parser.hpp>

#include <unordered_map>

using namespace black;

TEST_CASE("Terms manipulation") {
  alphabet sigma;

  auto c = sigma.constant(42);
  auto y = sigma.var("y");

  auto n1 = next(c);
  auto n2 = next(y);

  function f{"f"};

  term app1 = f(n1, n2);
  term app2 = n1 + n2;

  REQUIRE(sigma.from_id(app1.unique_id()) == app1);

  REQUIRE(app1 != app2);

  std::string s = app1.match(
    [](constant)    { return "c1"; },
    [](variable)    { return "v1"; },
    [](application a) { 
      black_assert(a.arguments().size() > 0);
      return a.arguments()[0].match(
        [](constant) { return "c2"; },
        [](variable) { return "v2"; },
        [](application) { return "a2"; },
        [](next) { return "n2"; },
        [](wnext) { return "w2"; },
        [](prev) { return "p2"; },
        [](wprev) { return "wp2"; }
      );
    },
    [](next)        { return "n1"; },
    [](wnext)        { return "w1"; },
    [](prev)       { return "p1"; },
    [](wprev)       { return "wp1"; }
  );

  REQUIRE(s == "n2");

  relation r{"r"};

  atom a1 = r(app1, app2);
  atom a2 = app1 <= app2;

  REQUIRE(a1 != a2);

}

TEST_CASE("Terms in hash tables") {
  alphabet sigma;
  sigma.set_domain(sort::Int);
  
  variable x = sigma.var("x");
  variable y = sigma.var("y");

  std::unordered_map<term, std::string> tests = {
    {x, "x"},
    {y, "y"},
    {x + y, "x + y"}
  };

  REQUIRE(tests.find(x) != tests.end());
  REQUIRE(tests.find(y) != tests.end());
  REQUIRE(tests.find(x + y) != tests.end());
}

TEST_CASE("Test formulas") {
  alphabet sigma;
  sigma.set_domain(sort::Int);

  variable x1 = sigma.var("x1");
  variable x2 = sigma.var("x2");
  variable x3 = sigma.var("x3");
  function f{"f"};
  
  std::vector<formula> formulas = {
    G(x1 == x2) && F(f(x1) == x3),
    G(next(x1) == x1 + 1) && F(x1 == 42),
    x1 == 0 && X(prev(x1) == 0),
    wprev(x1) == 0,
    implies(x1 == 0, prev(x1) == 0)
  };

  for(formula frm : formulas) {
    DYNAMIC_SECTION("Formula: " << frm) {
      solver slv;
      slv.set_formula(frm);
      REQUIRE(slv.solve() == true);
    }
  }
}
