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
    object p = m.declare(
      {"p", function_type({sigma.integer_type()}, sigma.boolean_type())}
    );
    object x = m.declare({"x", sigma.integer_type()});

    REQUIRE(m.type_of(p(x)) == sigma.boolean_type());
  }

  SECTION("Definitions") {
    variable a = sigma.variable("a");
    
    object x = m.define({"x", sigma.integer_type(), sigma.integer(40)});
    object p = 
      m.define({
        "p", {{a, sigma.integer_type()}}, sigma.integer_type(),
        ite(a > sigma.integer(0), a + x, a - x)
      });
    m.define({a, sigma.real_type(), sigma.real(0.0)}); // this will be shadowed

    term ft = function_type({sigma.integer_type()}, sigma.integer_type());
    REQUIRE(m.lookup("p")->lookup()->type == ft);

    term t = p(sigma.integer(2));

    REQUIRE(m.type_of(t) == sigma.integer_type());
    
    REQUIRE(m.evaluate(t) == sigma.integer(42));


  }

  SECTION("Mixed declarations and definitions") {
    auto a = sigma.variable("a");

    object x = m.declare({"x", sigma.integer_type()});
    object p = 
      m.define({"p", {{a, sigma.integer_type()}}, sigma.integer_type(), a + x});

    term t = p(sigma.integer(40));

    REQUIRE(m.type_of(t) == sigma.integer_type());
    
    REQUIRE(m.evaluate(t) == sigma.integer(40) + x);
  }


  SECTION("Name lookup and resolution") {

    // variable x = sigma.variable("x");
    // variable y = sigma.variable("y");
    // variable z = sigma.variable("z");

    // m.define(x, sigma.integer_type(), y + z);
    // m.define(y, sigma.integer_type(), x - z);
    // m.declare(z, sigma.integer_type());

    // module m2 = m.resolved();

  }

}