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

    alphabet sigma;
    
    cvc5::solver slv(&sigma);

    SECTION("Declarations") {
        object x = slv.declare({"x", sigma.integer_type()});
        object y = slv.declare({"y", sigma.integer_type()});
        
        slv.require(x <= y);
        
        REQUIRE(slv.check() == true);
        
        slv.push();
        slv.require(x > y);

        REQUIRE(slv.check() == false);

        slv.pop();

        REQUIRE(slv.check() == true);

        REQUIRE(slv.check_assuming(x > y) == false);
    }

    SECTION("Definitions") 
    {    
        variable a = sigma.variable("a");

        object factor = slv.declare({"factor", sigma.integer_type()});
        object f = slv.define({"f", {{a, sigma.integer_type()}}, a * factor});
        
        slv.require(factor >= 2);
        slv.require(f(21) >= 42);

        REQUIRE(slv.check() == true);
        
        slv.require(f(21) < 42);

        REQUIRE(slv.check() == false);
    }

    SECTION("Declaration of functions/predicates") {

        variable x = sigma.variable("x");

        object p = slv.declare({
            "p", function_type({sigma.integer_type()}, sigma.boolean_type())
        });

        slv.require(forall({{x, sigma.integer_type()}}, p(x)));
        slv.require(!p(42));

        REQUIRE(slv.check() == false);

    }


}