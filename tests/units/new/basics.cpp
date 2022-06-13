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

#include <black/new/logic.hpp>

#include <string>
#include <type_traits>
#include <ranges>

using namespace std::literals;
using namespace black::new_api::logic;
using black::internal::identifier;

static_assert(black::internal::new_api::hierarchy<formula<LTL>>);
static_assert(black::internal::new_api::hierarchy<proposition>);
static_assert(black::internal::new_api::hierarchy<unary<LTL>>);
static_assert(black::internal::new_api::hierarchy<conjunction<LTL>>);
static_assert(black::internal::new_api::hierarchy<equal<FO>>);
static_assert(black::internal::new_api::storage_kind<proposition>);
static_assert(black::internal::new_api::storage_kind<unary<LTL>>);
static_assert(black::internal::new_api::storage_kind<conjunction<LTL>>);
static_assert(black::internal::new_api::hierarchy_element<conjunction<LTL>>);
static_assert(black::internal::new_api::hierarchy_element<equal<FO>>);

static_assert(
  std::is_same_v<black::internal::new_api::make_combined_fragment_t<LTL>, LTL>
);


TEST_CASE("New API") {

  black::new_api::logic::alphabet sigma;

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
    function f = sigma.function("f");

    REQUIRE(f.is<function>());

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
    comparison<FO> e = comparison<FO>(comparison<FO>::type::equal, x, x);
    quantifier<FO> f = quantifier<FO>(quantifier<FO>::type::forall, x, e);

    REQUIRE(e.left() == x);
    REQUIRE(e.right() == x);

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
    equal eq = equal(x, x);

    static_assert(std::is_same_v<decltype(c2), binary<Boolean>>);

    REQUIRE(b.is<boolean>());
    REQUIRE(p.is<proposition>());
    REQUIRE(u.is<negation<Boolean>>());
    REQUIRE(c.is<conjunction<Boolean>>());
    REQUIRE(c2.is<conjunction<Boolean>>());
    REQUIRE(eq.is<equal<FO>>());
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

    comparison lt = x < c;
    comparison le = x <= c;
    comparison gt = x > c;
    comparison ge = x >= c;
    comparison eq = x == c;
    comparison ne = x != c;
    formula feq = x == c;
    formula fne = x != c;
    comparison lt0 = x < 0;
    comparison le0 = x <= 0;
    comparison gt0 = x > 0;
    comparison ge0 = x >= 0;
    comparison eq0 = x == 0;
    comparison ne0 = x != 0;
    formula feq0 = x == 0;
    formula fne0 = x != 0;
    comparison lt0p = 0 < x;
    comparison le0p = 0 <= x;
    comparison gt0p = 0 > x;
    comparison ge0p = 0 >= x;
    comparison eq0p = 0 == x;
    comparison ne0p = 0 != x;
    formula feq0p = 0 == x;
    formula fne0p = 0 != x;
    comparison lt0f = x < 0.0;
    comparison le0f = x <= 0.0;
    comparison gt0f = x > 0.0;
    comparison ge0f = x >= 0.0;
    comparison eq0f = x == 0.0;
    comparison ne0f = x != 0.0;
    formula feq0f = x == 0.0;
    formula fne0f = x != 0.0;
    comparison lt0fp = 0.0 < x;
    comparison le0fp = 0.0 <= x;
    comparison gt0fp = 0.0 > x;
    comparison ge0fp = 0.0 >= x;
    comparison eq0fp = 0.0 == x;
    comparison ne0fp = 0.0 != x;
    formula feq0fp = 0.0 == x;
    formula fne0fp = 0.0 != x;

    REQUIRE(lt.is<comparison<FO>>());
    REQUIRE(le.is<comparison<FO>>());
    REQUIRE(gt.is<comparison<FO>>());
    REQUIRE(ge.is<comparison<FO>>());
    REQUIRE(eq.is<comparison<FO>>());
    REQUIRE(ne.is<comparison<FO>>());
    REQUIRE(feq.is<comparison<FO>>());
    REQUIRE(fne.is<comparison<FO>>());
    REQUIRE(lt0.is<comparison<FO>>());
    REQUIRE(le0.is<comparison<FO>>());
    REQUIRE(gt0.is<comparison<FO>>());
    REQUIRE(ge0.is<comparison<FO>>());
    REQUIRE(eq0.is<comparison<FO>>());
    REQUIRE(ne0.is<comparison<FO>>());
    REQUIRE(feq0.is<comparison<FO>>());
    REQUIRE(fne0.is<comparison<FO>>());
    REQUIRE(lt0p.is<comparison<FO>>());
    REQUIRE(le0p.is<comparison<FO>>());
    REQUIRE(gt0p.is<comparison<FO>>());
    REQUIRE(ge0p.is<comparison<FO>>());
    REQUIRE(eq0p.is<comparison<FO>>());
    REQUIRE(ne0p.is<comparison<FO>>());
    REQUIRE(feq0p.is<comparison<FO>>());
    REQUIRE(fne0p.is<comparison<FO>>());
    REQUIRE(lt0f.is<comparison<FO>>());
    REQUIRE(le0f.is<comparison<FO>>());
    REQUIRE(gt0f.is<comparison<FO>>());
    REQUIRE(ge0f.is<comparison<FO>>());
    REQUIRE(eq0f.is<comparison<FO>>());
    REQUIRE(ne0f.is<comparison<FO>>());
    REQUIRE(feq0f.is<comparison<FO>>());
    REQUIRE(fne0f.is<comparison<FO>>());
    REQUIRE(lt0fp.is<comparison<FO>>());
    REQUIRE(le0fp.is<comparison<FO>>());
    REQUIRE(gt0fp.is<comparison<FO>>());
    REQUIRE(ge0fp.is<comparison<FO>>());
    REQUIRE(eq0fp.is<comparison<FO>>());
    REQUIRE(ne0fp.is<comparison<FO>>());
    REQUIRE(feq0fp.is<comparison<FO>>());
    REQUIRE(fne0fp.is<comparison<FO>>());


    binary_term plus = x + c;
    binary_term minus = x - c;
    unary_term neg = -x;
    binary_term mult = x * c;
    binary_term div = x / c;
    binary_term plus0 = x + 0;
    binary_term minus0 = x - 0;
    binary_term mult0 = x * 0;
    binary_term div0 = x / 0;
    binary_term plus0p = 0 + x;
    binary_term minus0p = 0 - x;
    binary_term mult0p = 0 * x;
    binary_term div0p = 0 / x;
    binary_term plus0f = x + 0.0;
    binary_term minus0f = x - 0.0;
    binary_term mult0f = x * 0.0;
    binary_term div0f = x / 0.0;
    binary_term plus0fp = 0.0 + x;
    binary_term minus0fp = 0.0 - x;
    binary_term mult0fp = 0.0 * x;
    binary_term div0fp = 0.0 / x;

    REQUIRE(plus.is<binary_term<FO>>());
    REQUIRE(minus.is<binary_term<FO>>());
    REQUIRE(neg.is<unary_term<FO>>());
    REQUIRE(mult.is<binary_term<FO>>());
    REQUIRE(div.is<binary_term<FO>>());
    REQUIRE(plus0.is<binary_term<FO>>());
    REQUIRE(minus0.is<binary_term<FO>>());
    REQUIRE(mult0.is<binary_term<FO>>());
    REQUIRE(div0.is<binary_term<FO>>());
    REQUIRE(plus0p.is<binary_term<FO>>());
    REQUIRE(minus0p.is<binary_term<FO>>());
    REQUIRE(mult0p.is<binary_term<FO>>());
    REQUIRE(div0p.is<binary_term<FO>>());
    REQUIRE(plus0f.is<binary_term<FO>>());
    REQUIRE(minus0f.is<binary_term<FO>>());
    REQUIRE(mult0f.is<binary_term<FO>>());
    REQUIRE(div0f.is<binary_term<FO>>());
    REQUIRE(plus0fp.is<binary_term<FO>>());
    REQUIRE(minus0fp.is<binary_term<FO>>());
    REQUIRE(mult0fp.is<binary_term<FO>>());
    REQUIRE(div0fp.is<binary_term<FO>>());
    
  }

  SECTION("Complex formula") {
    variable x = sigma.variable("x");
    function f = sigma.function("f");

    formula complex = (x == 0 && G(wnext(x) == f(x) + 1) && F(x == 42));

    REQUIRE(complex.is<conjunction<LTLFO>>());
  }
  
  SECTION("Iteration over associative operators") {
    using namespace black::internal::new_api;

    boolean b = sigma.boolean(true);
    proposition p = sigma.proposition("p");
    variable x = sigma.variable("x");
    variable y = sigma.variable("y");

    conjunction<LTL> c = b && ((p && (b && p)) && b);
    addition<FO> sum = x + ((y + (x + y)) + x);

    using view_t = decltype(c.operands());
    STATIC_REQUIRE(std::input_or_output_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::input_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::forward_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::ranges::range<view_t>);
    STATIC_REQUIRE(std::ranges::view<view_t>);
    STATIC_REQUIRE(std::ranges::viewable_range<view_t>);

    std::vector<formula<LTL>> v1 = {b, p, b, p, b};
    std::vector<formula<LTL>> v2;

    for(auto f : c.operands())
      v2.push_back(f);

    REQUIRE(v1 == v2);
    
    std::vector<term<FO>> tv1 = {x, y, x, y, x};
    std::vector<term<FO>> tv2;

    for(auto f : sum.operands())
      tv2.push_back(f);

    REQUIRE(tv1 == tv2);
  }

  SECTION("Quantifier blocks") {
    using namespace black::internal::new_api;

    variable x = sigma.variable("x");
    variable y = sigma.variable("y");
    variable z = sigma.variable("z");
    variable w = sigma.variable("w");

    formula<FO> f = x == y && y == z && z == w;

    std::vector<variable> v = {x, y, z, w};

    static_assert(storage_kind<quantifier_block<FO>>);

    auto qb = quantifier_block<FO>(quantifier<FO>::type::exists, v, f);

    REQUIRE(qb.matrix() == f);

    quantifier<FO> q = qb;

    quantifier<FO> q2 = exists(x, exists(y, (exists(z, exists(w, f)))));

    REQUIRE(q == q2);

    exists<FO> eb = exists_block<FO>(v, f);
    
    REQUIRE(eb == q2);

    std::vector<variable> vars;
    for(auto var : q.block().variables()) {
      vars.push_back(var);
    }

    REQUIRE(v == vars);
    REQUIRE(q.block().matrix() == f);

    using view_t = decltype(qb.variables());
    STATIC_REQUIRE(std::input_or_output_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::input_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::forward_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::ranges::range<view_t>);
    STATIC_REQUIRE(std::ranges::view<view_t>);
    STATIC_REQUIRE(std::ranges::viewable_range<view_t>);

    formula<FO> qf = q;
    qf.match(
      [&](quantifier_block<FO> b) {
        std::vector<variable> bvars;
        for(auto var : q.block().variables()) {
          bvars.push_back(var);
        }
        REQUIRE(bvars == v);
        REQUIRE(b.matrix() == f);
      },
      [](otherwise) { REQUIRE(false); }
    );

    qf.match(
      [&](exists_block<FO> b) {
        std::vector<variable> bvars;
        for(auto var : q.block().variables()) {
          bvars.push_back(var);
        }
        REQUIRE(bvars == v);
        REQUIRE(b.matrix() == f);
      },
      [](otherwise) { REQUIRE(false); }
    );
  }
}
