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

using namespace std::literals;
using namespace black::new_api::syntax;
using black::internal::identifier;

static_assert(black::internal::new_api::hierarchy<formula<LTL>>);
static_assert(black::internal::new_api::hierarchy<proposition>);
static_assert(black::internal::new_api::hierarchy<unary<LTL>>);
static_assert(black::internal::new_api::hierarchy<conjunction<LTL>>);
static_assert(black::internal::new_api::hierarchy<equal>);
static_assert(black::internal::new_api::storage_kind<proposition>);
static_assert(black::internal::new_api::storage_kind<unary<LTL>>);
static_assert(black::internal::new_api::storage_kind<conjunction<LTL>>);
static_assert(black::internal::new_api::hierarchy_element<conjunction<LTL>>);
static_assert(black::internal::new_api::hierarchy_element<equal>);



TEST_CASE("New API") {

  black::new_api::syntax::alphabet sigma;

  SECTION("Formula deduplication") {
    REQUIRE(sigma.boolean(true) == sigma.boolean(true));

    REQUIRE(sigma.proposition("p") == sigma.proposition("p"));

    formula<LTL> c = conjunction<LTL>(sigma.boolean(true), sigma.boolean(true));
    formula<LTL> d = conjunction<LTL>(sigma.boolean(true), sigma.boolean(true));

    REQUIRE(c == d);
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

    unary<LTL> u = unary<LTL>(unary<LTL>::type::eventually, b);

    REQUIRE(u.argument() == b);

    REQUIRE(u.argument().is<boolean>());
    REQUIRE(u.argument().to<boolean>()->value() == true);

    REQUIRE(u.is<eventually<LTL>>());
    REQUIRE(u.to<eventually<LTL>>()->argument() == b);
    REQUIRE(u.to<eventually<LTL>>()->argument().is<boolean>());
    REQUIRE(u.to<eventually<LTL>>()->argument().to<boolean>()->value() == true);
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
      always<LTLP> a = always<LTL>(b);

      formula<LTLP> f = a;
      REQUIRE(f == a);

      f = n;
      REQUIRE(f == n);
    }

    SECTION("Storage kinds and hierarchy elements") {
      negation<LTL> n = negation<Boolean>(b);
      always<LTLP> a = always<LTL>(b);

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

    REQUIRE(bool(app == app2));
    REQUIRE(bool(app == app3));
  }

  SECTION("Quantifiers") {
    variable x = sigma.variable("x");
    atom<FO> e = atom<FO>(sigma.equal(), std::vector{x, x});
    quantifier<FO> f = quantifier<FO>(quantifier<FO>::type::forall, x, e);

    REQUIRE(e.rel() == sigma.equal());
    REQUIRE(e.terms() == std::vector<term<FO>>{x, x});

    REQUIRE(bool(f.var() == x));
    REQUIRE(f.matrix() == e);
  }
  
  SECTION("Deduction guides") {
    formula b = sigma.boolean(true);
    formula p = sigma.proposition("p");
    variable x = sigma.variable("x");
    unary u = unary<Boolean>(unary<Boolean>::type::negation, b);
    conjunction c = conjunction(p, b);
    binary c2 = conjunction(u, b);
    atom app = atom(sigma.equal(), std::vector{x, x});

    static_assert(std::is_same_v<decltype(c2), binary<Boolean>>);

    REQUIRE(b.is<boolean>());
    REQUIRE(p.is<proposition>());
    REQUIRE(u.is<negation<Boolean>>());
    REQUIRE(c.is<conjunction<Boolean>>());
    REQUIRE(c2.is<conjunction<Boolean>>());
    REQUIRE(app.is<atom<FO>>());
  }

  SECTION("Sugar for formulas") {
    boolean b = sigma.boolean(true);
    proposition p = sigma.proposition("p");

    negation n = !b;
    tomorrow x = X(b);
    w_tomorrow wx = w_tomorrow(b);
    yesterday y = Y(b);
    w_yesterday z = Z(b);
    always g = G(b);
    eventually f = F(b);
    once o = O(b);
    historically h = H(b);

    REQUIRE(n.is<negation<LTLP>>());
    REQUIRE(x.is<tomorrow<LTLP>>());
    REQUIRE(wx.is<w_tomorrow<LTLP>>());
    REQUIRE(y.is<yesterday<LTLP>>());
    REQUIRE(z.is<w_yesterday<LTLP>>());
    REQUIRE(g.is<always<LTLP>>());
    REQUIRE(f.is<eventually<LTLP>>());
    REQUIRE(o.is<once<LTLP>>());
    REQUIRE(h.is<historically<LTLP>>());

    conjunction c = b && p;
    disjunction d = b || p;
    implication i = implies(b, p);
    until u = U(b, p);
    release r = R(b, p);
    w_until wu = wU(b, p);
    s_release sr = sR(b, p);
    since s = S(b, p);
    triggered t = T(b, p);

    REQUIRE(c.is<conjunction<LTLP>>());
    REQUIRE(d.is<disjunction<LTLP>>());
    REQUIRE(i.is<implication<LTLP>>());
    REQUIRE(u.is<until<LTLP>>());
    REQUIRE(r.is<release<LTLP>>());
    REQUIRE(wu.is<w_until<LTLP>>());
    REQUIRE(sr.is<s_release<LTLP>>());
    REQUIRE(s.is<since<LTLP>>());
    REQUIRE(t.is<triggered<LTLP>>());
  }

  SECTION("Sugar for terms") {
    variable x = sigma.variable("x");
    constant c = constant{sigma.integer(42)};

    atom lt = x < c;
    atom le = x <= c;
    atom gt = x > c;
    atom ge = x >= c;
    atom eq = x == c;
    atom ne = x != c;
    formula feq = x == c;
    formula fne = x != c;
    atom lt0 = x < 0;
    atom le0 = x <= 0;
    atom gt0 = x > 0;
    atom ge0 = x >= 0;
    atom eq0 = x == 0;
    atom ne0 = x != 0;
    formula feq0 = x == 0;
    formula fne0 = x != 0;
    atom lt0p = 0 < x;
    atom le0p = 0 <= x;
    atom gt0p = 0 > x;
    atom ge0p = 0 >= x;
    atom eq0p = 0 == x;
    atom ne0p = 0 != x;
    formula feq0p = 0 == x;
    formula fne0p = 0 != x;
    atom lt0f = x < 0.0;
    atom le0f = x <= 0.0;
    atom gt0f = x > 0.0;
    atom ge0f = x >= 0.0;
    atom eq0f = x == 0.0;
    atom ne0f = x != 0.0;
    formula feq0f = x == 0.0;
    formula fne0f = x != 0.0;
    atom lt0fp = 0.0 < x;
    atom le0fp = 0.0 <= x;
    atom gt0fp = 0.0 > x;
    atom ge0fp = 0.0 >= x;
    atom eq0fp = 0.0 == x;
    atom ne0fp = 0.0 != x;
    formula feq0fp = 0.0 == x;
    formula fne0fp = 0.0 != x;

    REQUIRE(lt.is<atom<FO>>());
    REQUIRE(le.is<atom<FO>>());
    REQUIRE(gt.is<atom<FO>>());
    REQUIRE(ge.is<atom<FO>>());
    REQUIRE(eq.is<atom<FO>>());
    REQUIRE(ne.is<atom<FO>>());
    REQUIRE(feq.is<atom<FO>>());
    REQUIRE(fne.is<atom<FO>>());
    REQUIRE(lt0.is<atom<FO>>());
    REQUIRE(le0.is<atom<FO>>());
    REQUIRE(gt0.is<atom<FO>>());
    REQUIRE(ge0.is<atom<FO>>());
    REQUIRE(eq0.is<atom<FO>>());
    REQUIRE(ne0.is<atom<FO>>());
    REQUIRE(feq0.is<atom<FO>>());
    REQUIRE(fne0.is<atom<FO>>());
    REQUIRE(lt0p.is<atom<FO>>());
    REQUIRE(le0p.is<atom<FO>>());
    REQUIRE(gt0p.is<atom<FO>>());
    REQUIRE(ge0p.is<atom<FO>>());
    REQUIRE(eq0p.is<atom<FO>>());
    REQUIRE(ne0p.is<atom<FO>>());
    REQUIRE(feq0p.is<atom<FO>>());
    REQUIRE(fne0p.is<atom<FO>>());
    REQUIRE(lt0f.is<atom<FO>>());
    REQUIRE(le0f.is<atom<FO>>());
    REQUIRE(gt0f.is<atom<FO>>());
    REQUIRE(ge0f.is<atom<FO>>());
    REQUIRE(eq0f.is<atom<FO>>());
    REQUIRE(ne0f.is<atom<FO>>());
    REQUIRE(feq0f.is<atom<FO>>());
    REQUIRE(fne0f.is<atom<FO>>());
    REQUIRE(lt0fp.is<atom<FO>>());
    REQUIRE(le0fp.is<atom<FO>>());
    REQUIRE(gt0fp.is<atom<FO>>());
    REQUIRE(ge0fp.is<atom<FO>>());
    REQUIRE(eq0fp.is<atom<FO>>());
    REQUIRE(ne0fp.is<atom<FO>>());
    REQUIRE(feq0fp.is<atom<FO>>());
    REQUIRE(fne0fp.is<atom<FO>>());


    application plus = x + c;
    application minus = x - c;
    application neg = -x;
    application mult = x * c;
    application div = x / c;
    application plus0 = x + 0;
    application minus0 = x - 0;
    application mult0 = x * 0;
    application div0 = x / 0;
    application plus0p = 0 + x;
    application minus0p = 0 - x;
    application mult0p = 0 * x;
    application div0p = 0 / x;
    application plus0f = x + 0.0;
    application minus0f = x - 0.0;
    application mult0f = x * 0.0;
    application div0f = x / 0.0;
    application plus0fp = 0.0 + x;
    application minus0fp = 0.0 - x;
    application mult0fp = 0.0 * x;
    application div0fp = 0.0 / x;

    REQUIRE(plus.is<application<FO>>());
    REQUIRE(minus.is<application<FO>>());
    REQUIRE(neg.is<application<FO>>());
    REQUIRE(mult.is<application<FO>>());
    REQUIRE(div.is<application<FO>>());
    REQUIRE(plus0.is<application<FO>>());
    REQUIRE(minus0.is<application<FO>>());
    REQUIRE(mult0.is<application<FO>>());
    REQUIRE(div0.is<application<FO>>());
    REQUIRE(plus0p.is<application<FO>>());
    REQUIRE(minus0p.is<application<FO>>());
    REQUIRE(mult0p.is<application<FO>>());
    REQUIRE(div0p.is<application<FO>>());
    REQUIRE(plus0f.is<application<FO>>());
    REQUIRE(minus0f.is<application<FO>>());
    REQUIRE(mult0f.is<application<FO>>());
    REQUIRE(div0f.is<application<FO>>());
    REQUIRE(plus0fp.is<application<FO>>());
    REQUIRE(minus0fp.is<application<FO>>());
    REQUIRE(mult0fp.is<application<FO>>());
    REQUIRE(div0fp.is<application<FO>>());
    
  }

  SECTION("Complex formula") {
    variable x = sigma.variable("x");
    function f = sigma.function_symbol("f");

    formula complex = (x == 0 && G(wnext(x) == f(x) + 1) && F(x == 42));

    REQUIRE(complex.is<conjunction<LTLFO>>());
  }
  
}
