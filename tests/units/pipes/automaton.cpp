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

#include <cmath>

using namespace black;
using namespace black::support;
using namespace black::logic;



/* TEST_CASE("FOLTL to automaton") {

    SECTION("Temporal formula") {

        module Sigma;

        variable x = "x";

        object c = Sigma.declare("c", types::integer());
        object p = Sigma.declare("p", types::boolean());

        term phi = exists({{x, types::integer()}}, U((wX(c)+1) > x, (X(p))));
        
        Sigma.require(phi);

        pipes::transform encoding = pipes::automaton() | pipes::debug();
        module result = encoding(Sigma);
    }
    
    SECTION("Weak next operator conversion") {

        module Sigma;
        variable x = "x";

        object c = Sigma.declare("c", types::integer());
        object p = Sigma.declare("p", types::boolean());
        object q = Sigma.declare("q", types::boolean());

        term phi = atom(
            Sigma.declare(
                "P", types::function({types::integer(), types::boolean()}, types::boolean()),
                role::state,
                resolution::immediate
            ),
            {c, p, wX(q)}
        );

        Sigma.require(phi);

        pipes::transform encoding = pipes::automaton() | pipes::debug();
        module result = encoding(Sigma);
    }

    SECTION("Next vs weak next operator") {

        module Sigma;

        variable x = "x";

        object c = Sigma.declare("c", types::integer());
        object p = Sigma.declare("p", types::boolean());
        object q = Sigma.declare("q", types::boolean());
        
        term phi = atom(
            Sigma.declare(
                "P", types::function({types::integer(), types::boolean()}, types::boolean()),
                role::state,
                resolution::immediate
            ),
            {c, X(p), wX(q)}
        );   

        Sigma.require(phi);

        pipes::transform encoding = pipes::automaton() | pipes::debug();
        module result = encoding(Sigma);
    }
}

*/