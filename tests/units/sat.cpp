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

#include <catch.hpp>
#include <black/solver/solver.hpp>
#include <black/sat/solver.hpp>

TEST_CASE("SAT backends") {

  std::vector<std::string> backends = {
    "z3", "mathsat", "cmsat", "cvc5"
  };

  black::alphabet sigma;
  black::scope xi{sigma};

  auto p = sigma.proposition("p");
  auto q = sigma.proposition("q");

  for(auto backend : backends) {
    DYNAMIC_SECTION("SAT backend: " << backend) {
      if(black::sat::solver::backend_exists(backend)) {
        auto slv = black::sat::solver::get_solver(backend, xi);

        REQUIRE(slv->value(p) == black::tribool::undef);
        slv->assert_formula(p);
        REQUIRE(slv->value(p) == black::tribool::undef);

        REQUIRE(slv->is_sat());
        REQUIRE(slv->value(p) == true);
        REQUIRE(slv->value(q) == black::tribool::undef);

        slv->clear();
        slv->assert_formula(!p);
        
        REQUIRE(slv->is_sat());
        REQUIRE(slv->value(p) == false);
      }
    }
  }
  
}
