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
#include <black/solvers/cvc5>

#include <cassert> // for the standard `assert` macro

using namespace black;
using namespace black::logic;

int main() {

    module mod;

    variable f = "f";
    variable n = "n";

    object fact = mod.define(
        f, {{n, types::integer()}}, types::integer(),
        ite(n == 1, 1, n * f(n - 1)),
        resolution::delayed
    );

    mod.resolve(recursion::allowed);

    object x = mod.declare("x", types::integer());

    mod.require(fact(x) == 3628800);

    solvers::solver slv = solvers::cvc5();

    assert(slv.check(mod) == true);

    assert(slv.value(x) == 10);

    return 0;
}