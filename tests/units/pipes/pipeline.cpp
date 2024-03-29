//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2024 Nicola Gigante
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

#include <black/support>
#include <black/logic>
#include <black/pipes>
#include <black/solvers/cvc5>

using namespace black;
using namespace black::support;
using namespace black::logic;

TEST_CASE("Pipeline") {

    SECTION("Basics") {

        module mod;

        object x = mod.declare("x", integer_type());

        mod.require(x == 0);

        pipes::transform t = pipes::id() | pipes::id();

        module mod2 = t(mod);

        REQUIRE(mod2 == mod);

        mod.require(x == 42);

        mod2 = t(mod);

        REQUIRE(mod2 == mod);

        solvers::solver slv = pipes::id() | solvers::unsat();

        REQUIRE(slv.check(mod) == false);
    }

    SECTION("With a real solver") {
        module mod;

        object x = mod.declare("x", integer_type());
        object y = mod.declare("y", integer_type());

        mod.require(x > y);

        solvers::solver slv = pipes::id() | pipes::id() | solvers::cvc5();

        REQUIRE(slv.check(mod) == true);

        mod.require(x < y);

        REQUIRE(slv.check(mod) == false);

    }
}