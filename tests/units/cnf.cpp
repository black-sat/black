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

#include <catch.hpp>

#include <black/logic/logic.hpp>
#include <black/logic/parser.hpp>
#include <black/logic/prettyprint.hpp>
#include <black/solver/solver.hpp>
#include <black/logic/cnf.hpp>
#include <black/internal/debug/random_formula.hpp>

using namespace black::logic;


TEST_CASE("CNF Translation")
{
  alphabet sigma;

  SECTION("remove_booleans()") {
    using namespace black_internal::cnf;

    boolean top = sigma.top();
    boolean bot = sigma.bottom();
    proposition p = sigma.proposition("p");
    proposition q = sigma.proposition("q");

    REQUIRE(remove_booleans(!top) == bot);
    REQUIRE(remove_booleans(!!top) == top);
    REQUIRE(remove_booleans(!!p) == p);
    
    REQUIRE(remove_booleans(top && p) == p);
    REQUIRE(remove_booleans(bot && p) == bot);
    REQUIRE(remove_booleans(p && top) == p);
    REQUIRE(remove_booleans(p && bot) == bot);
    REQUIRE(remove_booleans(top && top) == top);
    REQUIRE(remove_booleans(p && q) == (p && q));
    
    REQUIRE(remove_booleans(top || p) == top);
    REQUIRE(remove_booleans(bot || p) == p);
    REQUIRE(remove_booleans(p || top) == top);
    REQUIRE(remove_booleans(p || bot) == p);
    REQUIRE(remove_booleans(top || top) == top);
    REQUIRE(remove_booleans(p || q) == (p || q));
    
    REQUIRE(remove_booleans(implies(top, p)) == p);
    REQUIRE(remove_booleans(implies(bot, p)) == top);
    REQUIRE(remove_booleans(implies(p, top)) == top);
    REQUIRE(remove_booleans(implies(p, bot)) == !p);
    REQUIRE(remove_booleans(implies(top, top)) == top);
    REQUIRE(remove_booleans(implies(p, q)) == implies(p, q));
    
    REQUIRE(remove_booleans(iff(top, p)) == p);
    REQUIRE(remove_booleans(iff(bot, p)) == !p);
    REQUIRE(remove_booleans(iff(p, top)) == p);
    REQUIRE(remove_booleans(iff(p, bot)) == !p);
    REQUIRE(remove_booleans(iff(top, top)) == top);
    REQUIRE(remove_booleans(iff(p, q)) == iff(p, q));
    
    
  }

  SECTION("CNF of random formulas") {
    std::mt19937 gen((std::random_device())());

    std::vector<std::string> symbols = {
      "p1", "p2", "p3", "p4", "p5", "p6",
      "p7", "p8", "p9", "p10",
    };

    proposition p = sigma.proposition("p");
    proposition q = sigma.proposition("q");

    std::vector<formula<propositional>> tests = {
      p && q, p || q, implies(p, q), iff(p, q),
      !p, !(p && q), !(p || q), !implies(p, q), !iff(p, q)
    };

    black::solver s;

    for(formula f : tests) 
    { 
      DYNAMIC_SECTION("Formula: " << to_string(f)) {
        formula<propositional> fc = to_formula(sigma, black::to_cnf(f));
        s.set_formula(!implies(fc,f));

        INFO("CNF: " << to_string(fc));
        REQUIRE(!s.solve());
      }      
    }   
  }
  
}
