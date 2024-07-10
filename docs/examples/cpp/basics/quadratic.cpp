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

#include <black/logic>
#include <black/backends/cvc5>

#include <cassert> // for the standard `assert` macro
#include <iostream>

using namespace black;
using namespace black::logic;

int main() {

    module mod;

    object x = mod.declare("x", types::real());
    object a = mod.define("a",  types::real(),   1.0);
    object b = mod.define("b",  types::real(),  -8.0);
    object c = mod.define("c",  types::real(),  16.0);

    mod.require(a * x * x + b * x + c == 0.0);

    solvers::solver slv = solvers::cvc5();

    slv.set(solvers::option::logic, "QF_NRA");

    support::tribool result = slv.check(mod);
    
    assert(result == true);

    assert(slv.model().has_value());

    assert(slv.model()->value(x).has_value());
    assert(slv.model()->value(x) == 4.0);

    mod.require(x != 4.0);

    assert(slv.check(mod) == false);

    return 0;
}