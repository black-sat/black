//
// BLACK - Bounded Ltl sAtisfiability ChecKer
//
// (C) 2023 Nicola Gigante
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

TEST_CASE("Terms") {

  using namespace black::logic;
  
  alphabet sigma;

  auto p = sigma.symbol("p");
  auto q = sigma.symbol("q");

  negation n1 = !p;
  negation n2 = !q;    

  REQUIRE_FALSE(n1 == n2);
  REQUIRE(n1 != n2);

  term e = n1 == n2;

  REQUIRE(cast<equal>(e));

  atom a = p(e);

  REQUIRE(a.head() == p);

}

TEST_CASE("Modules") {

  using namespace black::logic;
  
  alphabet sigma;

  auto p = sigma.symbol("p");
  auto x = sigma.symbol("x");
  auto y = sigma.symbol("y");

  module m(&sigma);

  SECTION("Declarations") {
    m.declare(p, {sigma.integer_type()}, sigma.boolean_type());
    m.declare(x, sigma.integer_type());
    m.declare(y, sigma.real_type());

    scope::result<term> r1 = m.type_of(p(x));
    scope::result<term> r2 = m.type_of(p(y));

    REQUIRE(r1 == sigma.boolean_type());
    REQUIRE(!r2);
  }

  SECTION("Definitions") {
    auto a = sigma.symbol("a");
    
    m.define(p, {{a, sigma.integer_type()}}, 
      ite(a > sigma.integer(0), a + x, a - x)
    );
    m.define(x, sigma.integer(40));
    m.define(a, sigma.real(0.0)); // this will be shadowed

    term t = p(sigma.integer(2));

    scope::result<term> r1 = m.type_of(t);

    REQUIRE(r1.has_value());
    REQUIRE(r1 == sigma.integer_type());
    
    scope::result<term> r2 = m.value_of(t);
    REQUIRE(r2.has_value());
    REQUIRE(r2 == sigma.integer(42));

  }

  SECTION("Mixed declarations and definitions") {
    auto a = sigma.symbol("a");

    m.define(p, {{a, sigma.integer_type()}}, a + x);
    m.declare(x, sigma.integer_type());

    term t = p(sigma.integer(40));

    scope::result<term> r1 = m.type_of(t);
    REQUIRE(r1.has_value());
    REQUIRE(r1 == sigma.integer_type());
    
    scope::result<term> r2 = m.value_of(t);
    REQUIRE(!r2.has_value());

  }


}