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

#include <catch2/catch.hpp>

using namespace black;

TEST_CASE("Rountrip of parser and pretty-printer")
{
  alphabet sigma;

  atom p = sigma.var("p");
  atom q = sigma.var("q");

  std::vector<formula> tests = {
    p, !p, X(p), F(p), G(p), P(p), H(p), XF(p), GF(p), XG(p),
    p && q, p || q, U(p,q), S(p,q), R(p,q), T(p,q),
    p && (X(U(p,q)) || XF(!q)),
    p && implies(Y(S(p,q)), GF(!p)),
    U(p, !(GF(q))),
    !(iff(p || q, !q && p))
  };

  for(formula f : tests) {
    DYNAMIC_SECTION("Roundtrip for formula: " << f) {
      auto result = parse_formula(sigma, to_string(f));

      REQUIRE(result.has_value());
      CHECK(*result == f);
    }
  }
}
