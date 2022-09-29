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

TEST_CASE("Solver")
{
  alphabet sigma; // testing move constructor and assignment
  alphabet sigma_{std::move(sigma)};

  sigma = std::move(sigma_);

  SECTION("Propositional formulas") {

    std::vector<std::string> backends = {
      "z3", "mathsat", "cmsat", "minisat", "cvc5"
    };

    for(auto backend : backends) {
      DYNAMIC_SECTION("Backend: " << backend) {
        if(black::sat::solver::backend_exists(backend)) {
          black::solver slv;
          REQUIRE(slv.sat_backend() == BLACK_DEFAULT_BACKEND);
          REQUIRE(slv.solve() == true);
          
          slv.set_sat_backend(backend);

          auto p = sigma.proposition("p");
          auto q = sigma.proposition("q");

          std::vector<formula> tests = {
            !p && iff(!X(p), F(G(p))) && implies(p, q), p || !p,
            iff(p, q), iff(sigma.top(), p), !W(p, q),
            iff(p, sigma.top()), iff(sigma.top(), sigma.top()),
            implies(sigma.top(), p), 
            implies(sigma.top(), sigma.top())
          };

          for(auto f : tests) {
            DYNAMIC_SECTION("Formula: " << to_string(f)) {
              slv.set_formula(f);
              REQUIRE(!slv.model().has_value());

              REQUIRE(slv.solve());
            }
          }
        }
      }
    }
  }

  SECTION("First-order formulas") {

    std::vector<std::string> backends = {
      "z3", "mathsat", "cvc5"
    };

    for(auto backend : backends) {
      DYNAMIC_SECTION("Backend: " << backend) {
        if(black::sat::solver::backend_exists(backend)) {
          black::solver slv;
          REQUIRE(slv.sat_backend() == BLACK_DEFAULT_BACKEND);
          REQUIRE(slv.solve() == true);
          
          slv.set_sat_backend(backend);

          auto x = sigma.variable("x", sigma.integer_sort());
          auto rel = sigma.relation("r", seq<sort>{{sigma.integer_sort()}});

          std::vector<formula> tests = {
            G(x > 0), F(x == 1), F(-x == -x), !rel(prev(x)), rel(wprev(x)),
            rel(next(x)), rel(wnext(x)), X(prev(x) == x), X(wprev(x) == x)
          };

          for(auto f : tests) {
            DYNAMIC_SECTION("Formula: " << to_string(f)) {
              slv.set_formula(f);
              REQUIRE(!slv.model().has_value());

              REQUIRE(slv.solve());
            }
          }
        }
      }
    }
  }

  SECTION("Quantified formulas") {

    std::vector<std::string> backends = { "z3", "cvc5" };

    for(auto backend : backends) {
      DYNAMIC_SECTION("Backend: " << backend) {
        if(black::sat::solver::backend_exists(backend)) {
          variable x = sigma.variable("x", sigma.integer_sort());
          variable y = sigma.variable("y", sigma.integer_sort());
          variable z = sigma.variable("z", sigma.integer_sort());
          proposition p = sigma.proposition("p");
          function func = sigma.function(
            "f", sigma.integer_sort(), {sigma.integer_sort()}
          );
          
          std::vector<formula> tests = {
            forall(x, x == x),
            X(forall(x, x == x)),
            exists_block({x,y}, next(z) + 2 != y),
            exists_block({x,y}, sigma.top()),
            exists_block({x,y}, !p),
            !forall_block({x,y}, x != y),
            !exists(x, func(x) == x),
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

  SECTION("Solver syntax errors") {

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
        auto result = parse_formula(sigma, sigma.integer_sort(), s);

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

}
