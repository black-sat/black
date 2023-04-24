//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2022 Nicola Gigante
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

#include <black/logic/logic.hpp>
#include <black/sdd/sdd.hpp>

#include <catch.hpp>

#include <random>
#include <iostream>

TEST_CASE("automata") {

  namespace sdd = black::sdd;

  black::alphabet sigma;
  sdd::manager *mgr = new sdd::manager{&sigma};

  sdd::variable p = mgr->variable(sigma.proposition("p"));
  sdd::variable q = mgr->variable(sigma.proposition("q"));

  sdd::node n = p && !p;
  sdd::node v = p || !p;

  sdd::node n2 = p && !p;

  sdd::node n3 = p && q;

  REQUIRE(n.is_unsat());
  REQUIRE(v.is_valid());
  REQUIRE(iff(n,n2).is_valid());
  REQUIRE(exists(p, n3).is_sat());
  REQUIRE(forall(p, n3).is_unsat());

}