//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#include <black/logic/logic.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/solver/solver.hpp>
#include <black/sat/solver.hpp>

#include <catch.hpp>

using namespace black::logic::fragments::LTLPFO;

TEST_CASE("Sorts and domains") {
  alphabet sigma;

  auto x = sigma.variable("x");
  auto c1 = sigma.variable("c1");
  auto c2 = sigma.variable("c2");
  auto c3 = sigma.variable("c3");
  auto c4 = sigma.variable("c4");

  named_sort s = sigma.named_sort("example");

  std::vector<formula> tests = {
    exists({x[s]}, distinct(std::vector<term>{c1, c2, c3, c4, x})),
    !forall({x[s]}, x == c1 || x == c2 || x == c3 || x == c4)
  };

  scope xi{sigma};
  xi.push(s, make_domain({c1, c2, c3, c4}));

  REQUIRE(xi.domain(s));
  REQUIRE(xi.sort(c1));

  std::vector<std::string> backends = { "z3", "cvc5" };

  for(auto backend : backends) {
    DYNAMIC_SECTION("Backend: " << backend) {
      if(!black::sat::solver::backend_exists(backend))
        continue;

      for(auto test : tests) {
        DYNAMIC_SECTION("Formula: " << to_string(test)) {
          REQUIRE(black::solver::check_syntax(test, [](auto){ }));

          black::solver slv;
          slv.set_sat_backend(backend);
          REQUIRE(xi.type_check(test, std::nullopt, [](auto) { }));
          auto result = slv.solve(xi, test);
          REQUIRE(result == false);
        }
      }
    }
  }
}
