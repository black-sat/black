//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2019 Luca Geatti
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

#include <catch.hpp>

#include <black/support/config.hpp>
#include <black/logic/logic.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/solver/solver.hpp>
#include <black/sat/solver.hpp>

using namespace black;

TEST_CASE("Testing solver")
{
  alphabet sigma;
  black::solver slv;

  SECTION("Basic solver usage") {
    REQUIRE(slv.sat_backend() == BLACK_DEFAULT_BACKEND);
    REQUIRE(slv.solve() == true);

    auto p = sigma.proposition("p");
    
    formula f1 = !p && iff(!X(p), F(G(p))) && implies(p, !p);
    formula f2 = p && !p;

    slv.set_formula(f1);
    
    auto model = slv.model();
    REQUIRE(!model.has_value());

    REQUIRE(slv.solve());

    slv.set_formula(f2);
    REQUIRE(!slv.model().has_value());
  }
}

TEST_CASE("Quantified formulas") {

  std::vector<std::string> backends = { "z3", "cvc5" };

  for(auto backend : backends) {
    DYNAMIC_SECTION("Backend: " << backend) {
      if(black::sat::solver::backend_exists(backend)) {
        alphabet sigma;
        sigma.set_default_sort(sigma.integer_sort());

        variable x = sigma.variable("x");
        variable y = sigma.variable("y");
        variable z = sigma.variable("z");
        proposition p = sigma.proposition("p");
        
        std::vector<formula> tests = {
          forall(x, x == x),
          exists_block({x,y}, next(z) + 2 != y),
          exists_block({x,y}, sigma.top()),
          exists_block({x,y}, !p),
          !forall_block({x,y}, x != y),
          exists(x, X(x == y)),
          exists(x, wX(x == y)),
          exists(x, X(Y(x == 0))),
          exists(x, X(Z(x == 0)))
        };

        for(formula f : tests) {
          DYNAMIC_SECTION("Test formula: " << to_string(f)) {
            solver slv;
            slv.set_sat_backend(backend);
            slv.set_formula(f);

            REQUIRE(slv.solve());
          }
        }
      }
    }
  }
}

TEST_CASE("Solver syntax errors") {

  alphabet sigma;
  std::vector<std::string> tests = {
    "f(x) & f(x,y)", "f(x) = 2 & f(x)", "f(x) & f(x) = 2",
    "f(x) = 2 & f(x,y) = 2", "f(x) + 2 = 2 & f(x)", "f(x) & f(x) + 2 = 2",
    "next(x + y) = 2", "exists x . next(x) = x", 
    "wnext(x + y) = 2", "exists x . wnext(x) = x",
    "exists x . (wnext(x) = x || x = 0)", "exists x . F(x = 0)"
  };

  for(std::string s : tests) {
    DYNAMIC_SECTION("Test formula: " << s) 
    {
      auto result = parse_formula(sigma, s);

      REQUIRE(result.has_value());

      bool error = false;
      bool ok = solver::check_syntax(*result, [&](std::string){ 
        error = true;
      });

      REQUIRE(!ok);
      REQUIRE(error);
    }
  }

}
