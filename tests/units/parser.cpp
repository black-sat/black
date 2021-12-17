//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Nicola Gigante
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

#include <ostream>

#include <black/logic/alphabet.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>

#include <catch2/catch.hpp>

#include <iostream>

using namespace black;

TEST_CASE("Syntax errors") {

  alphabet sigma;
  std::vector<std::string> tests = {
    "F", "F(p U)", "p || q &&", "(p && q", "(", "F(p - q)", "F(p -)", 
    "F(p(x,))", "x = 1000000000000000000000", "x = 1.", "x x", "x % 1",
    "exists . p", "exists x y z +", "exists x . (x =)"
  };

  for(std::string s : tests) {
    DYNAMIC_SECTION("Test formula: " << s) 
    {
      bool error = false;
      auto result = parse_formula(sigma, s, [&](std::string) {
        error = true;
      });

      REQUIRE(error);
      if(result)
        std::cout << *result << "\n";
      REQUIRE(!result.has_value());
    }
  }

}

TEST_CASE("Roundtrip of parser and pretty-printer")
{
  alphabet sigma;

  proposition p = sigma.prop("p");
  proposition q = sigma.prop("q");
  variable x = sigma.var("x");
  variable y = sigma.var("y");
  variable z = sigma.var("z");

  function f{"f"};
  relation r{"r"};

  std::vector<formula> tests = {
    p, !p, X(p), F(p), G(p), O(p), H(p), XF(p), GF(p), XG(p),
    p && q, p || q, U(p,q), S(p,q), R(p,q), T(p,q),
    p && (X(U(p,q)) || XF(!q)),
    p && implies(Y(S(p,q)), GF(!p)),
    U(p, !(GF(q))),
    !(iff(p || q, !q && p)),
    exists({x,y,z}, f(x + 2, y) + 2 == (y + sigma.constant(1.5)) && y == z),
    forall({x,y,z}, r(x,-y) && next(x) == wnext(y) && y == z) && r(x,y)
  };

  for(formula f : tests) {
    DYNAMIC_SECTION("Roundtrip for formula: " << f) {
      auto result = parse_formula(sigma, to_string(f));

      REQUIRE(result.has_value());
      CHECK(*result == f);
    }
  }
}
