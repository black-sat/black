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

#include <black/new/formula.hpp>
#include <string>
#include <type_traits>
#include <iostream>

using namespace std::literals;
using namespace black::internal::new_api;
using black::internal::identifier;

//
// Runtime tests
//
TEST_CASE("New API") {

  alphabet sigma;

  SECTION("Formula deduplication") {
    REQUIRE(sigma.boolean(true) == sigma.boolean(true));

    REQUIRE(sigma.proposition("p") == sigma.proposition("p"));

    REQUIRE(
      conjunction<LTL>(sigma.boolean(true), sigma.boolean(true)) ==
      conjunction<LTL>(sigma.boolean(true), sigma.boolean(true))
    );
  }

  SECTION("Leaf storage kinds") {
    static_assert(!std::is_constructible_v<formula<LTL>, variable>);
    static_assert(!std::is_assignable_v<formula<LTL>, variable>);

    boolean b = sigma.boolean(true);
    REQUIRE(b.value() == true);

    proposition p = sigma.proposition("p");
    REQUIRE(p.label() == "p");

    variable x = sigma.variable("x");
    REQUIRE(x.label() == "x");

    formula<LTL> f = p;

    REQUIRE(f.is<proposition>());
    REQUIRE(!f.is<boolean>());

    f = b;

    REQUIRE(f.is<boolean>());
    REQUIRE(!f.is<proposition>());
  }

  SECTION("Storage kinds") {
    boolean b = sigma.boolean(true);

    unary<LTL> u = unary<LTL>(unary<LTL>::type::negation, b);

    REQUIRE(u.argument() == b);

    REQUIRE(u.argument().is<boolean>());
    REQUIRE(u.argument().to<boolean>()->value() == true);

    REQUIRE(u.is<negation<LTL>>());
    REQUIRE(u.to<negation<LTL>>()->argument() == b);
    REQUIRE(u.to<negation<LTL>>()->argument().is<boolean>());
    REQUIRE(u.to<negation<LTL>>()->argument().to<boolean>()->value() == true);
  }

  SECTION("Hierarchy elements") {
    static_assert(!std::is_constructible_v<unary<LTL>, conjunction<LTL>>);
    static_assert(!std::is_assignable_v<unary<LTL>, conjunction<LTL>>);

    boolean b = sigma.boolean(true);

    negation<LTL> n = negation<LTL>(b);
    always<LTL> a = always<LTL>(b);

    REQUIRE(n.argument() == b);
    REQUIRE(n.argument().is<boolean>());
    REQUIRE(n.argument().to<boolean>()->value() == true);

    REQUIRE(a.argument() == b);
    REQUIRE(a.argument().is<boolean>());
    REQUIRE(a.argument().to<boolean>()->value() == true);

    unary<LTL> u = n;
    REQUIRE(u == n);

    u = a;
    REQUIRE(u == a);    
  }

  SECTION("Conversions between different syntaxes") {
    static_assert(!std::is_constructible_v<unary<FO>, until<LTL>>);
    static_assert(!std::is_assignable_v<unary<FO>, until<LTL>>);
    static_assert(!std::is_constructible_v<unary<LTL>, conjunction<LTLP>>);
    static_assert(!std::is_assignable_v<unary<LTL>, conjunction<LTLP>>);

    boolean b = sigma.boolean(true);
    
    SECTION("Storage kinds") {
      unary<LTL> n = unary<LTL>(unary<LTL>::type::negation, b);
      unary<LTL> a = unary<LTL>(unary<LTL>::type::always, b);
    
      formula<LTLP> f = n;
      REQUIRE(f == n);

      f = a;
      REQUIRE(f == a);
    }

    SECTION("Hierarchy elements") {
      negation<LTL> n = negation<LTL>(b);
      always<LTL> a = always<LTL>(b);

      formula<LTLP> f = a;
      REQUIRE(f == a);

      f = n;
      REQUIRE(f == n);
    }

    SECTION("Storage kinds and hierarchy elements") {
      negation<LTL> n = negation<LTL>(b);
      always<LTL> a = always<LTL>(b);

      unary<LTLP> u = n;
      REQUIRE(u == n);

      u = a;
      REQUIRE(u == a);
    }

    SECTION("is<>, to<> and from<>") {
      negation<LTL> n = negation<LTL>(sigma.boolean(true));

      REQUIRE(n.is<negation<LTL>>());
      REQUIRE(n.is<negation<LTLP>>());
      REQUIRE(!n.is<negation<Boolean>>());
      REQUIRE(!n.is<variable>());
    }
  }

  SECTION("Atoms and applications") {
    function<FO> f = sigma.function_symbol("f");

    REQUIRE(f.is<function_symbol>());

    variable x = sigma.variable("x");
    variable y = sigma.variable("y");

    application<FO> app = application<FO>(f, std::vector{x,y});
    REQUIRE(app.func() == f);
    REQUIRE(app.terms() == std::vector<term<FO>>{x,y});

    application<FO> app2 = f(x, y);

    REQUIRE(app.func() == f);
    REQUIRE(app.terms() == std::vector<term<FO>>{x,y});

    application<FO> app3 = f(std::vector{x, y});

    REQUIRE(app.func() == f);
    REQUIRE(app.terms() == std::vector<term<FO>>{x,y});

    REQUIRE(app == app2);
    REQUIRE(app == app3);
  }

  SECTION("Quantifiers") {
    variable x = sigma.variable("x");
    atom<FO> e = atom<FO>(sigma.equal(), std::vector{x, x});
    quantifier<FO> f = quantifier<FO>(quantifier<FO>::type::forall, x, e);

    REQUIRE(e.rel() == sigma.equal());
    REQUIRE(e.terms() == std::vector<term<FO>>{x, x});

    REQUIRE(f.var() == x);
    REQUIRE(f.matrix() == e);
  }
  
}
