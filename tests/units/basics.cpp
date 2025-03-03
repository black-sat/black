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

#include <string>
#include <type_traits>
#include <ranges>

using namespace std::literals;
using namespace black::logic;
using black_internal::identifier;

static_assert(black::logic::hierarchy<formula>);
static_assert(black::logic::hierarchy<proposition>);
static_assert(black::logic::hierarchy<unary>);
static_assert(black::logic::hierarchy<conjunction>);
static_assert(black::logic::hierarchy<equal>);
static_assert(black::logic::storage_kind<proposition>);
static_assert(black::logic::storage_kind<unary>);
static_assert(black::logic::storage_kind<conjunction>);
static_assert(black::logic::hierarchy_element<conjunction>);
static_assert(black::logic::hierarchy_element<equal>);

static_assert(
  std::is_same_v<black::logic::make_combined_fragment_t, LTL>
);


TEST_CASE("New API") {

  black::logic::alphabet sigma;

  SECTION("Formula deduplication") {
    REQUIRE(sigma.boolean(true) == sigma.boolean(true));

    REQUIRE(sigma.proposition("p") == sigma.proposition("p"));

    formula c = conjunction(sigma.boolean(true), sigma.boolean(true));
    formula d = conjunction(sigma.boolean(true), sigma.boolean(true));

    REQUIRE(c == d);
  }

  SECTION("Leaf storage kinds") {
    static_assert(!std::is_constructible_v<formula, variable>);
    static_assert(!std::is_assignable_v<formula, variable>);

    boolean b = sigma.boolean(true);
    REQUIRE(b.value() == true);

    proposition p = sigma.proposition("p");
    REQUIRE(p.name() == "p");

    variable x = sigma.variable("x");
    REQUIRE(x.name() == "x");

    formula f = p;

    REQUIRE(f.is<proposition>());
    REQUIRE(!f.is<boolean>());

    f = b;

    REQUIRE(f.is<boolean>());
    REQUIRE(!f.is<proposition>());
  }

  SECTION("Storage kinds") {
    boolean b = sigma.boolean(true);

    unary u = unary(unary::type::eventually{}, b);

    REQUIRE(u.argument() == b);

    REQUIRE(u.argument().is<boolean>());
    REQUIRE(u.argument().to<boolean>()->value() == true);

    REQUIRE(u.is<eventually>());
    REQUIRE(u.to<eventually>()->argument() == b);
    REQUIRE(u.to<eventually>()->argument().is<boolean>());
    REQUIRE(u.to<eventually>()->argument().to<boolean>()->value() == true);
  }

  SECTION("Hierarchy elements") {
    static_assert(!std::is_constructible_v<unary, conjunction>);
    static_assert(!std::is_assignable_v<unary, conjunction>);

    boolean b = sigma.boolean(true);

    negation n = negation(b);
    always a = always(b);

    REQUIRE(n.argument() == b);
    REQUIRE(n.argument().is<boolean>());
    REQUIRE(n.argument().to<boolean>()->value() == true);

    REQUIRE(a.argument() == b);
    REQUIRE(a.argument().is<boolean>());
    REQUIRE(a.argument().to<boolean>()->value() == true);

    unary u = n;
    REQUIRE(u == n);

    u = a;
    REQUIRE(u == a);    
  }

  SECTION("Use of fragment type enums values") {
    unary::type t1 = unary::type::always{};
    unary::type t2 = unary::type::negation{};

    REQUIRE(t1 != t2);

    std::vector<unary::type> v = {
      unary::type::always{}, unary::type::negation{}, 
      unary::type::eventually{}
    };

    REQUIRE(v[0] == t1);
  }

  SECTION("Conversions between different syntaxes") {
    static_assert(!std::is_constructible_v<unary, until>);
    static_assert(!std::is_assignable_v<unary, until>);
    static_assert(!std::is_constructible_v<unary, conjunction>);
    static_assert(!std::is_assignable_v<unary, conjunction>);

    boolean b = sigma.boolean(true);
    
    SECTION("Storage kinds") {
      unary n = unary(unary::type::negation{}, b);
      unary a = unary(unary::type::always{}, b);
    
      formula f = n;
      REQUIRE(f == n);

      f = a;
      REQUIRE(f == a);
    }

    SECTION("Hierarchy elements") {
      negation n = negation(b);
      always a = always(b);

      formula f = a;
      REQUIRE(f == a);

      f = n;
      REQUIRE(f == n);
    }

    SECTION("Storage kinds and hierarchy elements") {
      negation n = negation<propositional>(b);
      always a = always(b);

      unary u = n;
      REQUIRE(u == n);

      u = a;
      REQUIRE(u == a);
    }

    SECTION("is<>, to<> and from<>") {
      negation n = negation(sigma.boolean(true));

      REQUIRE(n.is<negation>());
      REQUIRE(n.is<negation>());
      REQUIRE(n.is<negation<propositional>>());
      REQUIRE(!n.is<unary_term>());

      proposition p = sigma.proposition("p");
      formula u1 = G(p);
      formula u2 = Y(sigma.proposition("p"));
      formula u3 = G(Y(sigma.proposition("p")));

      auto x = sigma.variable("x");
      auto y = sigma.variable("y");
      auto z = sigma.variable("z");

      std::vector<variable> vars = {x, y, z};
      std::vector<term> sums = {x + x, y + y, z + z};

      auto e1 = equal(vars);
      auto e2 = equal(sums);

      using vars_fragment = decltype(e1)::syntax;

      REQUIRE(u1.to<formula>().has_value());
      REQUIRE(!u2.to<formula>().has_value());
      REQUIRE(!u3.to<formula>().has_value());
      
      REQUIRE(!e2.to<equal<vars_fragment>>().has_value());

      REQUIRE(u1.node_type().to<formula::type>().has_value());
      REQUIRE(!u2.node_type().to<formula::type>().has_value()); 
    }
  }

  SECTION("Tuple-like access") {
    auto p = sigma.proposition("p");
    auto q = sigma.proposition("q");

    auto c = p && q;

    auto [l, r] = c;

    REQUIRE(l == p);
    REQUIRE(r == q);

    REQUIRE(get<0>(c) == p);
    REQUIRE(get<1>(c) == q);

    STATIC_REQUIRE(std::tuple_size_v<decltype(c)> == 2);
  }

  SECTION("Atoms and applications") {
    function f = sigma.function("f");

    REQUIRE(f.is<function>());

    variable x = sigma.variable("x");
    variable y = sigma.variable("y");
    std::vector<term> variables = {x,y};

    application app = application(f, variables);
    REQUIRE(app.func() == f);
    REQUIRE(variables == app.terms());

    application app2 = f(x,y);

    REQUIRE(app.func() == f);
    REQUIRE(app.terms() == std::vector<term>{x,y});

    application app3 = f(std::vector{x, y});

    REQUIRE(app.func() == f);
    REQUIRE(app.terms() == std::vector<term>{x,y});

    sort s = sigma.integer_sort();
    application app4 = f(x[s], y[s]);

    std::vector<var_decl> decls = {x[s], y[s]};
    application app5 = f(decls);

    REQUIRE(bool(app == app2));
    REQUIRE(bool(app == app3));
    REQUIRE(bool(app == app4));
    REQUIRE(bool(app == app5));

    application app6 = f(1, 2, 3);

    REQUIRE(app6.terms()[0] == 1);
    REQUIRE(app6.terms()[1] == 2);
    REQUIRE(app6.terms()[2] == 3);
  }

  SECTION("Quantifiers") {
    variable x = sigma.variable("x");
    sort s = sigma.integer_sort();
    comparison e = 
      comparison(comparison::type::greater_than{}, x, x);
    quantifier f = 
      quantifier(quantifier::type::forall{}, {x[s]}, e);

    REQUIRE(e.left() == x);
    REQUIRE(e.right() == x);

    REQUIRE(bool(f.variables() == std::vector{x[s]}));
    REQUIRE(f.matrix() == e);
  }
  
  SECTION("Deduction guides") {
    formula b = sigma.boolean(true);
    formula p = sigma.proposition("p");
    variable x = sigma.variable("x");
    unary u = unary<propositional>(unary<propositional>::type::negation{}, b);
    conjunction c = conjunction(p, b);
    binary c2 = conjunction(u, b);
    equal eq = equal(std::vector<term>{x, x});

    static_assert(std::is_same_v<decltype(c2), binary<propositional>>);

    REQUIRE(b.is<boolean>());
    REQUIRE(p.is<proposition>());
    REQUIRE(u.is<negation<propositional>>());
    REQUIRE(c.is<conjunction<propositional>>());
    REQUIRE(c2.is<conjunction<propositional>>());
    REQUIRE(eq.is<equal>());
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

    REQUIRE(n.is<negation>());
    REQUIRE(x.is<tomorrow>());
    REQUIRE(wx.is<w_tomorrow>());
    REQUIRE(y.is<yesterday>());
    REQUIRE(z.is<w_yesterday>());
    REQUIRE(g.is<always>());
    REQUIRE(f.is<eventually>());
    REQUIRE(o.is<once>());
    REQUIRE(h.is<historically>());

    conjunction c = b && p;
    disjunction d = b || p;
    implication i = implies(b, p);
    until u = U(b, p);
    release r = R(b, p);
    w_until wu = W(b, p);
    s_release sr = M(b, p);
    since s = S(b, p);
    triggered t = T(b, p);

    REQUIRE(c.is<conjunction>());
    REQUIRE(d.is<disjunction>());
    REQUIRE(i.is<implication>());
    REQUIRE(u.is<until>());
    REQUIRE(r.is<release>());
    REQUIRE(wu.is<w_until>());
    REQUIRE(sr.is<s_release>());
    REQUIRE(s.is<since>());
    REQUIRE(t.is<triggered>());
  }

  SECTION("Sugar for terms") {
    variable x = sigma.variable("x");
    constant c = constant{sigma.integer(42)};

    comparison lt = x < c;
    comparison le = x <= c;
    comparison gt = x > c;
    comparison ge = x >= c;
    equality eq = x == c;
    equality ne = x != c;
    formula feq = x == c;
    formula fne = x != c;
    comparison lt0 = x < 0;
    comparison le0 = x <= 0;
    comparison gt0 = x > 0;
    comparison ge0 = x >= 0;
    equality eq0 = x == 0;
    equality ne0 = x != 0;
    formula feq0 = x == 0;
    formula fne0 = x != 0;
    comparison lt0p = 0 < x;
    comparison le0p = 0 <= x;
    comparison gt0p = 0 > x;
    comparison ge0p = 0 >= x;
    equality eq0p = 0 == x;
    equality ne0p = 0 != x;
    formula feq0p = 0 == x;
    formula fne0p = 0 != x;
    comparison lt0f = x < 1.0;
    comparison le0f = x <= 1.0;
    comparison gt0f = x > 1.0;
    comparison ge0f = x >= 1.0;
    equality eq0f = x == 1.0;
    equality ne0f = x != 1.0;
    formula feq0f = x == 0.0;
    formula fne0f = x != 0.0;
    comparison lt0fp = 0.0 < x;
    comparison le0fp = 0.0 <= x;
    comparison gt0fp = 0.0 > x;
    comparison ge0fp = 0.0 >= x;
    equality eq0fp = 0.0 == x;
    equality ne0fp = 0.0 != x;
    formula feq0fp = 0.0 == x;
    formula fne0fp = 0.0 != x;

    REQUIRE(lt.is<comparison>());
    REQUIRE(le.is<comparison>());
    REQUIRE(gt.is<comparison>());
    REQUIRE(ge.is<comparison>());
    REQUIRE(eq.is<equality>());
    REQUIRE(ne.is<equality>());
    REQUIRE(feq.is<equality>());
    REQUIRE(fne.is<equality>());
    REQUIRE(lt0.is<comparison>());
    REQUIRE(le0.is<comparison>());
    REQUIRE(gt0.is<comparison>());
    REQUIRE(ge0.is<comparison>());
    REQUIRE(eq0.is<equality>());
    REQUIRE(ne0.is<equality>());
    REQUIRE(feq0.is<equality>());
    REQUIRE(fne0.is<equality>());
    REQUIRE(lt0p.is<comparison>());
    REQUIRE(le0p.is<comparison>());
    REQUIRE(gt0p.is<comparison>());
    REQUIRE(ge0p.is<comparison>());
    REQUIRE(eq0p.is<equality>());
    REQUIRE(ne0p.is<equality>());
    REQUIRE(feq0p.is<equality>());
    REQUIRE(fne0p.is<equality>());
    REQUIRE(lt0f.is<comparison>());
    REQUIRE(le0f.is<comparison>());
    REQUIRE(gt0f.is<comparison>());
    REQUIRE(ge0f.is<comparison>());
    REQUIRE(eq0f.is<equality>());
    REQUIRE(ne0f.is<equality>());
    REQUIRE(feq0f.is<equality>());
    REQUIRE(fne0f.is<equality>());
    REQUIRE(lt0fp.is<comparison>());
    REQUIRE(le0fp.is<comparison>());
    REQUIRE(gt0fp.is<comparison>());
    REQUIRE(ge0fp.is<comparison>());
    REQUIRE(eq0fp.is<equality>());
    REQUIRE(ne0fp.is<equality>());
    REQUIRE(feq0fp.is<equality>());
    REQUIRE(fne0fp.is<equality>());

    REQUIRE((x == x && true) == true);
    REQUIRE((true && x == x) == true);
    REQUIRE((x == x || true) == true);
    REQUIRE((true || x == x) == true);


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
    binary_term plus0f = x + 1.0;
    binary_term minus0f = x - 1.0;
    binary_term mult0f = x * 1.0;
    binary_term div0f = x / 1.0;
    binary_term plus0fp = 0.0 + x;
    binary_term minus0fp = 0.0 - x;
    binary_term mult0fp = 0.0 * x;
    binary_term div0fp = 0.0 / x;

    REQUIRE(plus.is<binary_term>());
    REQUIRE(minus.is<binary_term>());
    REQUIRE(neg.is<unary_term>());
    REQUIRE(mult.is<binary_term>());
    REQUIRE(div.is<binary_term>());
    REQUIRE(plus0.is<binary_term>());
    REQUIRE(minus0.is<binary_term>());
    REQUIRE(mult0.is<binary_term>());
    REQUIRE(div0.is<binary_term>());
    REQUIRE(plus0p.is<binary_term>());
    REQUIRE(minus0p.is<binary_term>());
    REQUIRE(mult0p.is<binary_term>());
    REQUIRE(div0p.is<binary_term>());
    REQUIRE(plus0f.is<binary_term>());
    REQUIRE(minus0f.is<binary_term>());
    REQUIRE(mult0f.is<binary_term>());
    REQUIRE(div0f.is<binary_term>());
    REQUIRE(plus0fp.is<binary_term>());
    REQUIRE(minus0fp.is<binary_term>());
    REQUIRE(mult0fp.is<binary_term>());
    REQUIRE(div0fp.is<binary_term>());
    
  }

  SECTION("Complex formula") {
    variable x = sigma.variable("x");
    function f = sigma.function("f");

    formula complex = (x == 0 && G(wnext(x) == f(x) + 1) && F(x == 42));

    REQUIRE(complex.is<conjunction>());
  }
  
  SECTION("Iteration over associative operators") {
    using namespace black_internal;

    boolean b = sigma.boolean(true);
    proposition p = sigma.proposition("p");
    variable x = sigma.variable("x");
    variable y = sigma.variable("y");

    conjunction c = b && ((p && (b && p)) && b);
    addition sum = x + ((y + (x + y)) + x);

    using view_t = decltype(c.operands());
    STATIC_REQUIRE(std::input_or_output_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::input_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::forward_iterator<view_t::const_iterator>);
    STATIC_REQUIRE(std::ranges::range<view_t>);
    STATIC_REQUIRE(std::ranges::view<view_t>);
    STATIC_REQUIRE(std::ranges::viewable_range<view_t>);

    std::vector<formula> v1 = {b, p, b, p, b};
    std::vector<formula> v2;

    for(auto f : c.operands())
      v2.push_back(f);

    REQUIRE(v1 == v2);
    
    std::vector<term> tv1 = {x, y, x, y, x};
    std::vector<term> tv2;

    for(auto f : sum.operands())
      tv2.push_back(f);

    REQUIRE(tv1 == tv2);
  }

  SECTION("has_any_element_of()")
  {
    using namespace black::logic;

    proposition p = sigma.proposition("p");
    variable x = sigma.variable("x");
    boolean top = sigma.top();

    sort s = sigma.integer_sort();

    REQUIRE(has_any_element_of(
      p && !p && x > x && exists({x[s]}, x > x) && F(F(top)),
      syntax_element::boolean, quantifier::type::forall{}
    ));
  }

  SECTION("big_and, big_or, etc...") {
    using namespace black::logic;

    auto p1 = sigma.proposition("p1");
    auto p2 = sigma.proposition("p2");
    auto p3 = sigma.proposition("p3");
    auto p4 = sigma.proposition("p4");

    std::vector<proposition> v = {p1, p2, p3, p4};
    auto t = big_and(sigma, v, [&](auto p) {
      return !p;
    });

    REQUIRE(t == (((!p1 && !p2) && !p3) && !p4));
  }
}
