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
  
  variable p = "p";
  variable q = "q";

  boolean b = true;

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
  
  module m;

  SECTION("Declarations") 
  {
    object p = m.declare(
      {"p", function_type({integer_type()}, boolean_type())}
    );
    object x = m.declare({"x", integer_type()});

    REQUIRE(type_of(p(x)) == boolean_type());
  }

  SECTION("Definitions") {
    variable a = variable("a");
    
    term tint = 42;

    object x = m.define({"x", integer(40)});
    object p = m.define({"p", {{a, integer_type()}}, ite(a > 0, a + x, a - x)});
    object aobj = m.define({a, real(0.0)}); // this will be shadowed

    REQUIRE(type_of(x) == integer_type());
    REQUIRE(type_of(aobj) == real_type());

    term ft = function_type({integer_type()}, integer_type());
    term ty = type_of(p);
    REQUIRE(ty == ft);

    term t = p(2);

    REQUIRE(type_of(t) == integer_type());
    
    REQUIRE(evaluate(t) == 42);
  }

  SECTION("Mixed declarations and definitions") {
    auto a = variable("a");

    object x = m.declare({"x", integer_type()});
    object p = 
      m.define({"p", {{a, integer_type()}}, integer_type(), a + x});

    term t = p(40);

    REQUIRE(type_of(t) == integer_type());
    
    REQUIRE(evaluate(t) == 40 + x);
  }


  SECTION("Mutually recursive scope and type inference") {

    variable f = "f";
    variable a = "a";
    variable x = "x";
    variable y = "y";

    object fobj = 
      m.define(
        {f, {{a, integer_type()}}, a + x + y}, resolution::delayed
      );

    object xobj = m.define({x, integer_type(), y}, resolution::delayed);
    object yobj = m.define({y, integer_type(), x}, resolution::delayed);
    
    m.resolve(scope::recursive);

    REQUIRE(m.lookup(f) == fobj);
    REQUIRE(m.lookup(x) == xobj);
    REQUIRE(m.lookup(y) == yobj);
    
    term ft = function_type({integer_type()}, integer_type());
    REQUIRE(fobj.lookup()->type == ft);
    REQUIRE(xobj.lookup()->value == yobj);
    REQUIRE(yobj.lookup()->value == xobj);

    object wrongf = m.define(
      {f, {{a, integer_type()}}, f(a)},
      resolution::delayed
    );

    m.resolve(scope::recursive);

    term wrongft = function_type({integer_type()}, inferred_type());
    REQUIRE(wrongf.lookup()->type == wrongft);
  }

}