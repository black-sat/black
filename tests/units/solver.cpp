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

#include <catch2/catch.hpp>

#include <black/support/config.hpp>
#include <black/logic/formula.hpp>
#include <black/solver/solver.hpp>

using namespace black;

TEST_CASE("Testing solver")
{
  alphabet sigma;
  black::solver slv;

  SECTION("Basic solver usage") {
    REQUIRE(slv.sat_backend() == BLACK_DEFAULT_BACKEND);
    REQUIRE(slv.solve() == true);

    auto p = sigma.prop("p");
    
    formula f1 = !p && iff(!X(p), FG(p)) && implies(p, !p);
    formula f2 = p && !p;

    slv.set_formula(f1);
    
    auto model = slv.model();
    REQUIRE(!model.has_value());

    REQUIRE(slv.solve());

    slv.set_formula(f2);
    REQUIRE(!slv.model().has_value());
  }
}
