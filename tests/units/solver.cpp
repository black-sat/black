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

  scope xi{sigma};

  SECTION("Propositional formulas") {

    std::vector<std::string> backends = {
      "z3", "mathsat", "cmsat", "minisat", "cvc5"
    };

    for(auto backend : backends) {
      DYNAMIC_SECTION("Backend: " << backend) {
        if(black::sat::solver::backend_exists(backend)) {
          black::solver slv;
          REQUIRE(slv.sat_backend() == BLACK_DEFAULT_BACKEND);
          
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
              REQUIRE(!slv.model().has_value());
              REQUIRE(slv.solve(xi, f));
            }
          }
        }
      }
    }
  }

  SECTION("First-order formulas") {
    
    xi.set_default_sort(sigma.integer_sort());

    std::vector<std::string> backends = {
      "z3", "mathsat", "cvc5"
    };

    for(auto backend : backends) {
      DYNAMIC_SECTION("Backend: " << backend) {
        if(black::sat::solver::backend_exists(backend)) {
          black::solver slv;
          REQUIRE(slv.sat_backend() == BLACK_DEFAULT_BACKEND);
          
          slv.set_sat_backend(backend);

          auto x = sigma.variable("x");
          auto rel = sigma.relation("r");

          xi.declare(rel, {sigma.integer_sort()});

          std::vector<formula> tests = {
            G(x > 0), F(x == 1), F(-x == -x), !rel(prev(x)), rel(wprev(x)),
            rel(next(x)), rel(wnext(x)), X(prev(x) == x), X(wprev(x) == x)
          };

          for(auto f : tests) {
            DYNAMIC_SECTION("Formula: " << to_string(f)) {
              REQUIRE(!slv.model().has_value());
              REQUIRE(slv.solve(xi, f));
            }
          }
        }
      }
    }
  }

  SECTION("Querying the first-order models") {

    auto x = sigma.variable("x");
    auto y = sigma.variable("y");
    auto r = sigma.relation("r");
    auto s = sigma.relation("s");

    xi.declare(x, sigma.integer_sort());
    xi.declare(y, sigma.integer_sort());
    xi.declare(r, {sigma.integer_sort()});
    xi.declare(s, {sigma.integer_sort()});

    std::vector<std::string> backends = {
      "z3", "mathsat", "cvc5"
    };

    for(auto backend : backends) {
      DYNAMIC_SECTION("Backend: " << backend) {
        if(black::sat::solver::backend_exists(backend)) {
          
          black::solver slv;

          slv.solve(xi, x == 10 && r(x) && !r(y));
          REQUIRE(slv.model());

          REQUIRE(slv.model()->value(x == 10, 0));
          REQUIRE(!slv.model()->value(x == 11, 0));
          REQUIRE(slv.model()->value(x > 9, 0));
          REQUIRE(!slv.model()->value(x > 11, 0));
          REQUIRE(slv.model()->value(r(x), 0));
          REQUIRE(!slv.model()->value(r(y), 0));
        }
      }
    }
  }

  SECTION("Quantified formulas") {

    std::vector<std::string> backends = { "z3", "cvc5" };

    for(auto backend : backends) {
      DYNAMIC_SECTION("Backend: " << backend) {
        if(black::sat::solver::backend_exists(backend)) {
          variable x = sigma.variable("x");
          variable y = sigma.variable("y");
          variable z = sigma.variable("z");
          proposition p = sigma.proposition("p");
          function func = sigma.function("f");

          variable a = sigma.variable("a");
          variable b = sigma.variable("b");
          variable c = sigma.variable("c");

          named_sort s = sigma.named_sort("sort");

          xi.declare(s, make_domain({a,b,c}));

          xi.declare(func, s, {s});
          xi.declare(x, s);
          xi.declare(y, s);
          xi.declare(z, s);
          
          std::vector<formula> tests = {
            forall({x[s]}, x == x),
            X(forall({x[s]}, x == x)),
            exists({x[s],y[s]}, func(next(z)) != y),
            exists({x[s],y[s]}, sigma.top()),
            exists({x[s],y[s]}, !p),
            !forall({x[s],y[s]}, x != y),
            !exists({x[s]}, func(x) == x),
            exists({x[s]}, X(x == y)),
            exists({x[s]}, wX(x == y)),
            exists({x[s]}, X(Y(x == a))),
            exists({x[s]}, X(Z(x == b)))
          };

          for(formula f : tests) {
            DYNAMIC_SECTION("Test formula: " << to_string(f)) {
              solver slv;
              slv.set_sat_backend(backend);

              REQUIRE(xi.type_check(f, [](auto) { }));

              REQUIRE(slv.solve(xi, f));
            }
          }
        }
      }
    }
  }

  SECTION("Solver syntax errors") {

    std::vector<std::string> tests = {
      "next(x + y) = 2", "wnext(x + y) = 2", "exists x : Int . F(x = 0)"
    };

    for(std::string s : tests) {
      DYNAMIC_SECTION("Test formula: " << s) 
      {
        xi.set_default_sort(sigma.integer_sort());

        auto result = parse_formula<logic::LTLPFO>(sigma, s);

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
