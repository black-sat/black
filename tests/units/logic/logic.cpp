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

  auto p = sigma.variable("p");
  auto q = sigma.variable("q");

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
  namespace support = black::support;
  
  alphabet sigma;

  module m(&sigma);

  SECTION("Declarations") 
  {
    variable p = m.declare("p", {sigma.integer_type()}, sigma.boolean_type());
    variable x = m.declare("x", sigma.integer_type());
    variable y = m.declare("y", sigma.real_type());

    term r1 = m.type_of(p(x));
    // term r2 = m.type_of(p(y));

    REQUIRE(r1 == sigma.boolean_type());
    // TODO: check r2 contains an error
  }

  SECTION("Definitions") {
    variable a = sigma.variable("a");
    
    variable x = m.define("x", sigma.integer_type(), sigma.integer(40));
    variable p = 
      m.define("p", {{a, sigma.integer_type()}}, sigma.integer_type(),
        ite(a > sigma.integer(0), a + x, a - x)
      );
    m.define(a, sigma.real_type(), sigma.real(0.0)); // this will be shadowed

    term t = p(sigma.integer(2));

    term r1 = m.type_of(t);
    REQUIRE(r1 == sigma.integer_type());
    
    term r2 = m.evaluate(t);
    REQUIRE(r2 == sigma.integer(42));


  }

  SECTION("Mixed declarations and definitions") {
    auto a = sigma.variable("a");

    variable x = m.declare("x", sigma.integer_type());
    variable p = 
      m.define("p", {{a, sigma.integer_type()}}, sigma.integer_type(), a + x);

    term t = p(sigma.integer(40));

    term r1 = m.type_of(t);
    REQUIRE(r1 == sigma.integer_type());
    
    term r2 = m.evaluate(t);
    REQUIRE(r2 == sigma.integer(40) + x);
  }

  SECTION("Term cache") {

    variable x = sigma.variable("x");

    m.cache().insert(x, 42);

    std::optional<int> v1 = m.cache().get<int>(x);
    std::optional<float> v2 = m.cache().get<float>(x);

    REQUIRE(v1.has_value());
    REQUIRE(!v2.has_value());
    REQUIRE(*v1 == 42);

  }

}