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

#include <catch.hpp>

#include <black/logic/logic.hpp>
#include <black/logic/utility.hpp>

#include <iostream>

TEST_CASE("Utility functions in the `black::logic` namespace")
{
  using namespace black::logic;
  alphabet sigma;

  proposition p = sigma.proposition("p");
  variable x = sigma.variable("x");
  boolean top = sigma.top();
  boolean bot = sigma.bottom();

  SECTION("has_element") {
    REQUIRE(has_any_elements_of(
      p && (!p && (x > x && (exists(x, x > x) && top))),
      syntax_element::boolean, syntax_element::forall
    ));
  }
  
  SECTION("remove_booleans()") {
    REQUIRE(remove_booleans(!top) == bot);
    REQUIRE(remove_booleans(!bot) == top);
    REQUIRE(remove_booleans(!!top) == top);
    REQUIRE(remove_booleans(!!bot) == bot);
    REQUIRE(remove_booleans(!!p) == p);

    REQUIRE(remove_booleans(top && p) == p);
    REQUIRE(remove_booleans(bot && p) == bot);

    REQUIRE(remove_booleans(top || p) == top);
    REQUIRE(remove_booleans(bot || p) == p);

    REQUIRE(remove_booleans(implies(top, p)) == p);
    REQUIRE(remove_booleans(implies(bot, p)) == top);
    REQUIRE(remove_booleans(implies(p, top)) == top);
    REQUIRE(remove_booleans(implies(p, bot)) == !p);
    REQUIRE(remove_booleans(implies(top,bot)) == bot);
    
    REQUIRE(remove_booleans(iff(p, top)) == p);
    REQUIRE(remove_booleans(iff(p, bot)) == !p);
  }
  
}
