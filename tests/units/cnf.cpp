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
  std::mt19937 gen((std::random_device())());

  std::vector<std::string> symbols = {
    "p1", "p2", "p3", "p4", "p5", "p6",
    "p7", "p8", "p9", "p10",
  };


  std::vector<formula<propositional>> tests;
  for(int i = 0 ; i < 30; ++i) {
    tests.push_back(black::random_boolean_formula(gen, sigma, 10, symbols));
  }

  black::solver s;

  for(formula f : tests) 
  { 
    SECTION("CNF of random formula: ") {
      formula<propositional> fc = to_formula(sigma, black::to_cnf(f));
      s.set_formula(!implies(fc,f));

      INFO("Formula: " << to_string(f));
      INFO("CNF: " << to_string(fc));
      REQUIRE(!s.solve());
    }
  }
}
