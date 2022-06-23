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
    "p 42", "p 0.1", ",", ".",
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
      auto result = parse_formula(sigma, s, [&](std::string) {
        error = true;
      });

      REQUIRE(error);
      REQUIRE(!result.has_value());
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

  std::vector<sort> sorts = {sigma.integer_sort(), sigma.real_sort()};

  for(auto s : sorts) {
    DYNAMIC_SECTION("Sort: " << to_string(s)) {
      sigma.set_default_sort(s);

      std::vector<formula> tests = {
        p, !p, X(p), F(p), G(p), O(p), H(p), X(F(p)), G(F(p)), X(G(p)),
        p && q, p || q, U(p,q), S(p,q), R(p,q), T(p,q),
        p && (X(U(p,q)) || X(F(!q))),
        p && implies(Y(S(p,q)), G(F(!p))),
        U(p, !(G(F(q)))),
        !(iff(p || q, !q && p)),
        (forall(x, x == x)) && p,
        (exists(x, x == x)) && p,
        exists_block({x,y,z}, g(x + 1.0, y) - 2.0 >= (y * 0.0) && y == z / 2),
        forall_block({x,y,z}, r(x,-y) && next(x) == wnext(y) && y != z) &&
          r(x,y),
        (x + y) * z > 0, -x == y,
        W(p, q), M(p, q), x < y, x <= y, 
        x + constant{sigma.zero()} == x, x * constant{sigma.one()} == x,
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
    
  }
  
}
