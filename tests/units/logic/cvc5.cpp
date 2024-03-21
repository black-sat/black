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
#include <black/backends/cvc5>

using namespace black::support;
using namespace black::logic;
using namespace black::backends;

TEST_CASE("cvc5") {

    module mod;

    cvc5::solver slv;

    SECTION("Declarations") {
        object x = mod.declare({"x", integer_type()});
        object y = mod.declare({"y", integer_type()});
        
        mod.require(x <= y);

        REQUIRE(slv.check(mod) == true);
        
        mod.require(x > y);

        REQUIRE(slv.check(mod) == false);
    }

    SECTION("Definitions") 
    {    
        variable a = "a";

        object k = mod.declare({"k", integer_type()});
        object f = mod.define({"f", {{a, integer_type()}}, a * k});

        mod.require(k >= 2);
        mod.require(f(21) >= 42);

        REQUIRE(slv.check(mod) == true);
        
        mod.require(f(21) < 42);

        REQUIRE(slv.check(mod) == false);
    }

    SECTION("Declaration of functions/predicates") {

        variable x = "x";

        object p = mod.declare({
            "p", function_type({integer_type()}, boolean_type())
        });

        mod.require(forall({{x, integer_type()}}, p(x)));
        mod.require(!p(42));

        REQUIRE(slv.check(mod) == false);

    }

    SECTION("Push/pop interface") {
        
        object x = mod.declare({"x", integer_type()});
        object y = mod.declare({"y", integer_type()});
        
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
        variable y = "y";

        object fact = mod.define(
            {
                f, {{x, integer_type()}}, integer_type(), 
                ite(x == 1, 1, x * f(x - 1)) 
            }, resolution::delayed
        );

        mod.resolve(scope::recursive);

        REQUIRE(slv.check(mod) == true);

        mod.require(fact(4) != 24);

        REQUIRE(slv.check(mod) == false);        
    }

    SECTION("Module imports") {

        module math;

        object pi = math.define({"pi", 3.14});

        module geometry;

        geometry.import(math);

        variable r = "r";

        object perimeter = 
            geometry.define({"perimeter", {{r, real_type()}}, 2.0 * r * pi});
        
        object area = 
            geometry.define({"area", {{r, real_type()}}, 2.0 * r * r * pi});

        variable x = "x";
        geometry.require(
            forall(
                {{x, real_type()}}, 
                implies(x > 1, perimeter(x) < area(x))
            )
        );

        REQUIRE(slv.check(geometry) == true);

    }

}