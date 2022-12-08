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

using namespace std::literals;

using namespace black::logic;

TEST_CASE("Pattern matching") {
  black::logic::alphabet sigma;

  SECTION("Matching on formulas") {
    boolean b = sigma.boolean(true);
    formula<propositional> n = negation<propositional>(b);

    std::string s = n.match(
      [](boolean) { return "boolean"s; },
      [](proposition) { return "proposition"s; },
      [](unary<propositional>, formula<propositional> arg) { 
        return "unary<propositional>("s + 
          arg.match(
            [](boolean) { return "boolean"; },
            [](proposition) { return "proposition"; },
            [](unary<propositional>) { return "unary<propositional>"; },
            [](binary<propositional>) { return "binary<propositional>"; } 
          ) + ")"; 
      },
      [](binary<propositional>) { return "binary<propositional>"s; }
    );

    REQUIRE(s == "unary<propositional>(boolean)");
  }

  SECTION("Matching on storage kinds") {
    boolean b = sigma.boolean(true);
    unary<LTL> n = negation<LTL>(b);

    std::string s = n.match(
      [](negation<LTL>) { return "negation"; },
      [](tomorrow<LTL>) { return "tomorrow"; },
      [](w_tomorrow<LTL>) { return "w_tomorrow"; },
      [](always<LTL>) { return "always"; },
      [](eventually<LTL>) { return "eventually"; }
    );

    REQUIRE(s == "negation");
  }

  SECTION("Matching with different syntaxes") {
    boolean b = sigma.boolean(true);
    formula<LTL> n = negation<LTL>(b);

    std::string s = n.match(
      [](boolean) { return "boolean"; },
      [](proposition) { return "proposition"; },
      [](unary<LTLP>) { return "unary<LTLP>"; },
      [](binary<LTLFO>) { return "binary<LTLFO>"; }
    );

    REQUIRE(s == "unary<LTLP>");
  }

  SECTION("Matching with restricted syntax") {
    boolean b = sigma.boolean(true);
    proposition p = sigma.proposition("p");

    formula<LTLP> f = (b && Y(p));

    std::string s = f.match(
      [](yesterday<LTLP>) { return "yesterday"; },
      [](only<propositional, LTLP> o) {
        return o.match(
          [](boolean) { return "boolean"; },
          [](proposition) { return "proposition"; },
          [](negation<LTLP>) { return "negation"; },
          [](disjunction<LTLP>) { return "disjunction"; },
          [](conjunction<LTLP>) { return "conjunction"; },
          [](implication<LTLP>) { return "implication"; },
          [](iff<LTLP>) { return "iff"; }
        );
      },
      [](otherwise) { return "otherwise"; }
    );

    REQUIRE(s == "conjunction");
  }

  SECTION("Otherwise") { 
    formula<LTL> f = sigma.boolean(true);

    bool ok = f.match(
      [](boolean) { return true; },
      [](otherwise) { return false; }
    );

    REQUIRE(ok);

    ok = f.match(
      [](proposition) { return false; },
      [](otherwise) { return true; }
    );

    REQUIRE(ok);
  }

  SECTION("Unpacking") {
    SECTION("Simple children") {
      boolean b = sigma.boolean(true);
      proposition p = sigma.proposition("p");

      conjunction<LTLP> c = (b && Y(p));

      auto [l, r] = c;

      REQUIRE(l == b);
      REQUIRE(r == Y(p));

      formula<LTLP> f = c;
      f.match(
        [&](conjunction<LTLP> conj, formula<LTLP> left, formula<LTLP> right) {
          REQUIRE(conj.left() == left);
          REQUIRE(conj.right() == right);

          REQUIRE(left == b);
          REQUIRE(right == Y(p));
        },
        [](otherwise) {
          REQUIRE(false);
        }
      );
    }
    
    SECTION("Children vector") {
      relation r = sigma.relation("r");
      std::vector<term<LTLFO>> vars = {
        sigma.variable("x"), sigma.variable("y")
      };
      
      atom<LTLFO> a = r(vars);

      formula<LTLFO> f = a;
      f.match(
        [&](atom<LTLFO> at, relation rel, auto const& terms) { 
          REQUIRE(at.rel() == rel);
          REQUIRE(at.terms() == terms);
          REQUIRE(terms == vars);
        },
        [](otherwise) {
          REQUIRE(false);
        }
      );
    }
  }

  SECTION("Generic lambdas as handlers") {
    formula f = sigma.top() && sigma.bottom();

    f.match(
      [](conjunction<propositional>, auto ...args) {
        REQUIRE(sizeof...(args) == 2);
      },
      [](otherwise) {
        REQUIRE(false);
      }
    );
  }

  SECTION("Matching on `fragment_type`") {
    using namespace black_internal;

    proposition p = sigma.proposition("p");
    unary<LTL> u = !p;

    unary<LTLP>::type t = u.node_type();

    REQUIRE(t == unary<LTL>::type::negation{});

    REQUIRE(u.node_type() == unary<LTL>::type::negation{});

    auto tn = u.node_type().to<unary<LTL>::type::negation>();
    REQUIRE(tn.has_value());

    REQUIRE(u.node_type().is<unary<LTL>::type::negation>());

    u = F(p);

    unary<LTL>::type result = u.node_type().match(
      [](unary<LTL>::type::negation) {
        return unary<LTL>::type::negation{};
      },
      [](unary<LTL>::type::always) {
        return unary<LTL>::type::eventually{};
      },
      [](unary<LTL>::type::eventually) {
        return unary<LTL>::type::always{};
      },
      [](unary<LTL>::type::tomorrow) {
        return unary<LTL>::type::w_tomorrow{};
      },
      [](unary<LTL>::type::w_tomorrow) {
        return unary<LTL>::type::tomorrow{};
      }
    );

    REQUIRE(result == unary<LTL>::type::always{});
  }

  SECTION("Common type") {

    #define REQUIRE_CT(x, y, T) \
      STATIC_REQUIRE( \
        std::is_convertible_v< \
          std::common_type_t<decltype(x),decltype(y)>, T \
        > && \
        std::is_convertible_v< \
          T, std::common_type_t<decltype(x),decltype(y)> \
        > \
      );

    boolean b = sigma.boolean(true);
    proposition p = sigma.proposition("p");
    unary<LTL> u = !b;
    conjunction<LTL> c = b && p;
    disjunction<LTL> d = b || p;
    binary<LTLP> bin = c;
    formula<LTL> f = u;

    using F = make_combined_fragment_t<
      make_singleton_fragment_t<syntax_element::proposition>, 
      make_singleton_fragment_t<syntax_element::boolean>
    >;

    REQUIRE_CT(b, p, formula<F>);

    REQUIRE_CT(u, b, formula<LTL>);
    REQUIRE_CT(bin, u, formula<LTLP>);
    REQUIRE_CT(f, u, formula<LTL>);
    REQUIRE_CT(f, bin, formula<LTLP>);
    REQUIRE_CT(c, d, binary<LTL>);
    REQUIRE_CT(c, bin, binary<LTLP>);
    REQUIRE_CT(b, c, formula<LTL>);
    REQUIRE_CT(p, p, proposition);
    REQUIRE_CT(u, u, unary<LTL>);
    REQUIRE_CT(f, f, formula<LTL>);
  }
}
