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

#include <black/logic/logic.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>

#include <catch.hpp>

using namespace black;

TEST_CASE("Syntax errors") {

  alphabet sigma;
  std::vector<std::string> tests = {
    "F", "F(p U)", "p || q &&", "(p && q", "(", "F(p - q)", "F(p -)", 
    "F({so\\}me\\\\th\\ing})",
    "F(p(x,))", "x = 1000000000000000000000", "x = 1.", "x x", "x % 1",
    "exists . p", "exists x y z +", "exists x . (x =)", "p(x",
    "x + y * = x", "F(-)", "next x", "next(x x", "wnext x", "wnext(x x",
    "prev x", "wprev(x x", "next(next(x +)) = x", "wnext(next(x +)) = x", 
    "p 42", "p 0.1", ",", ".", "x +", "x *", "x -", "x /", "x = -",
    "x = 0.0000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000000000"
    "0000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000000000000000000000000000000000000001"
  };

  for(std::string s : tests) {
    DYNAMIC_SECTION("Test formula: " << s) 
    {
      bool error = false;
      auto result = 
        parse_formula(sigma, s, [&](std::string) {
          error = true;
        });

      REQUIRE(error);
      REQUIRE(!result.has_value());
    }
  }

}

TEST_CASE("Associativity, precedence and constants as in Spot")
{
  alphabet sigma;

  proposition a = sigma.proposition("a");
  proposition b = sigma.proposition("b");
  proposition c = sigma.proposition("c");

  std::vector<std::pair<std::string, formula>> tests = {
    {"a -> b -> c", implies(a, implies(b, c))},
    {"a <-> b <-> c", iff(a, iff(b, c))},
    {"a U b U c", U(a, U(b, c))},
    {"a R b R c", R(a, R(b, c))},
    {"a W b W c", W(a, W(b, c))},
    {"a M b M c", M(a, M(b, c))},
    {"a S b S c", S(a, S(b, c))},
    {"a T b T c", T(a, T(b, c))},
    {"a -> b & c", implies(a, b && c)},
    {"a | b -> c", implies(a || b, c)},
    {"a <-> b -> c", iff(a, implies(b, c))},
    {"a -> b <-> c", iff(implies(a, b), c)},
    {"!true", !sigma.top()},
    {"FaLsE | TRUE", sigma.bottom() || sigma.top()},
    {"{true}", sigma.proposition("true")}
  };

  for(auto [text, f] : tests) {
    DYNAMIC_SECTION("Parsing formula: " << text) {
      auto result = parse_formula(sigma, text, [](auto error){
        INFO("parsing error: " << error);
        REQUIRE(false);
      });

      REQUIRE(result.has_value());
      INFO("parsed: " << to_string(*result));
      CHECK(*result == f);
      CHECK(*parse_formula(sigma, to_string(f), [](auto){}) == f);
    }
  }
}

TEST_CASE("Roundtrip of parser and pretty-printer")
{
  alphabet sigma;
  
  proposition p = sigma.proposition("p{}");
  proposition q = sigma.proposition("");
  variable x = sigma.variable("x\\");
  variable y = sigma.variable("Y");
  variable z = sigma.variable("\\z");

  function g = sigma.function("g");
  relation r = sigma.relation("r");

  sort s = sigma.integer_sort();

  std::vector<formula> tests = {
    p, !p, X(p), F(p), G(p), O(p), H(p), X(F(p)), G(F(p)), X(G(p)),
    p && q, p || q, U(p,q), S(p,q), R(p,q), T(p,q),
    p && (X(U(p,q)) || X(F(!q))),
    p && implies(Y(S(p,q)), G(F(!p))),
    U(p, !(G(F(q)))),
    !(iff(p || q, !q && p)),
    (forall({x[s]}, x == x)) && p,
    (exists({x[s]}, x == x)) && p,
    exists({x[s],y[s],z[s]}, 
      g(x + 1, y) - 2 >= (y * 0) && y == div(z, 2)),
    forall({x[s],y[s],z[s]}, 
      r(x,-y) && next(x) == wnext(y) && y != z) && r(x,y),
    (x + y) * z > 0, -x == y,
    W(p, q), M(p, q), x < y, x <= y, 
    x + 0 == x, x * 1 == x,
    next(prev(x)) == x, next(wprev(x)) == x
  };

  for(formula f : tests) {
    DYNAMIC_SECTION("Roundtrip for formula: " << to_string(f)) {
      auto result = parse_formula(sigma, to_string(f), [](auto error){
        INFO("parsing error: " << error);
        REQUIRE(false);
      });
      
      REQUIRE(result.has_value());

      INFO("parsed: " << to_string(*result));
      
      CHECK(*result == f);
    }
  }
}
    

TEST_CASE("Operator precedence")
{
  alphabet sigma;

  proposition p = sigma.proposition("p");
  proposition q = sigma.proposition("q");
  proposition r = sigma.proposition("r");
  proposition s = sigma.proposition("s");
  variable x = sigma.variable("x");
  variable y = sigma.variable("y");
  variable z = sigma.variable("z");
  variable w = sigma.variable("w");

  std::vector<std::pair<std::string, formula>> tests = {
    {"p & q U r | s", (p && U(q, r)) || s},
    {"p | q & r U s | p", (p || (q && U(r, s))) || p},
    {"p -> q & s", implies(p, q && s)},
    {"p -> q U r & s", implies(p, U(q, r) && s)},
    {"x - y * z + w = x", x - y * z + w == x},
    {"x * y - z / w + x = x", x * y - z / w + x == x},
    {"-x + y = x", -x + y == x},
    {"-x * y = x", -x * y == x}
  };

  for(auto [text, f] : tests) {
    DYNAMIC_SECTION("Precedence for formula: " << text) {
      auto result = parse_formula(sigma, text, [](auto error){
        INFO("parsing error: " << error);
        REQUIRE(false);
      });

      REQUIRE(result.has_value());

      INFO("parsed: " << to_string(*result));

      CHECK(*result == f);
    }
  }
}
