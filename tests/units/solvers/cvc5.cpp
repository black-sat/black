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
#include <black/ast/algorithms>
#include <black/solvers/cvc5>

using namespace black;
using namespace black::support;
using namespace black::logic;

TEST_CASE("cvc5") {

    module mod;

    solvers::solver slv = black::solvers::cvc5();

    SECTION("Declarations") {
        object x = mod.declare("x", types::integer());
        object y = mod.declare("y", types::integer());
        
        mod.require(x <= y);

        REQUIRE(slv.check(mod) == true);
        
        mod.require(x > y);

        REQUIRE(slv.check(mod) == false);
    }

    SECTION("Definitions") 
    {    
        decl a = {"a", types::integer()};

        object k = mod.declare("k", types::integer());
        object f = mod.define("f", {a}, a * k);

        mod.require(k >= 2);
        mod.require(f(21) >= 42);

        REQUIRE(slv.check(mod) == true);
        
        mod.require(f(21) < 42);

        REQUIRE(slv.check(mod) == false);
    }

    SECTION("Declaration of functions/predicates") {

        object p = mod.declare(
            "p", types::function({types::integer()}, types::boolean())
        );
        
        decl x = {"x", types::integer()};

        mod.require(forall(x, p(x)));
        mod.require(!p(42));

        REQUIRE(slv.check(mod) == false);

    }

    SECTION("Push/pop interface") {
        
        object x = mod.declare("x", types::integer());
        object y = mod.declare("y", types::integer());
        
        mod.require(x <= y);
        REQUIRE(slv.check(mod) == true);

        mod.push();
        mod.require(x > y);
        REQUIRE(slv.check(mod) == false);
        
        mod.pop();
        REQUIRE(slv.check(mod) == true);
    }

    SECTION("Recursive definitions") {
        variable x = "x";
        variable f = "f";

        object fact = mod.define(
            f, {{x, types::integer()}}, types::integer(), 
            ite(x == 1, 1, x * f(x - 1)),
            resolution::delayed
        );
        
        object a = mod.declare("a", types::integer());
        
        mod.resolve(recursion::allowed);
        
        mod.require(fact(a) == 24);
        
        REQUIRE(slv.check(mod) == true);

        REQUIRE(slv.value(a).has_value());
        REQUIRE(slv.value(a) == 4);
    }

    SECTION("Module imports") {

        module math;

        object pi = math.define("pi", 3.14);

        module geometry;

        geometry.import(math);

        decl r = {"r", types::real()};

        object perimeter = 
            geometry.define("perimeter", {r}, 2.0 * r * pi);
        
        object area = 
            geometry.define("area", {r}, 2.0 * r * r * pi);

        geometry.require(forall(r, implies(r > 1, perimeter(r) < area(r))));

        REQUIRE(slv.check(geometry) == true);

    }

    SECTION("Model values") {
        object x = mod.declare("x", types::integer());
        
        mod.require(x <= 0);
        mod.require(x >= 0);
        REQUIRE(slv.check(mod) == true);

        REQUIRE(slv.value(x).has_value());
        REQUIRE(slv.value(x) == 0);
    }

}

TEST_CASE("Example transform") {

    module mod;

    solvers::solver slv = solvers::cvc5();

    auto discretize = []() {
        return pipes::map(
            [](types::real) { return types::integer(); },
            [](real, double v) { return integer(uint64_t(v)); }
        );
    };

    
    object x = mod.declare("x", types::real());

    mod.push();
    mod.require(3.0 < x && x < 4.0);

    REQUIRE(slv.check(mod) == true);

    slv = discretize() | solvers::cvc5();

    REQUIRE(slv.check(mod) == false);
    
    mod.pop();
    mod.require(3.0 < x && x < 5.0);

    REQUIRE(slv.check(mod) == true);

    REQUIRE(slv.value(x) == 4);


}