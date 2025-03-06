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

using namespace black;

TEST_CASE("Pattern matching") {
  black::alphabet sigma;

  SECTION("Matching on formulas") {
    boolean b = sigma.boolean(true);
    formula n = negation(b);

    std::string s = n.match(
      [](boolean) { return "boolean"s; },
      [](proposition) { return "proposition"s; },
      [](unary, formula arg) { 
        return "unary("s + 
          arg.match(
            [](boolean) { return "boolean"; },
            [](proposition) { return "proposition"; },
            [](unary) { return "unary"; },
            [](binary) { return "binary"; } 
          ) + ")"; 
      },
      [](binary) { return "binary"s; }
    );

    REQUIRE(s == "unary(boolean)");
  }

  SECTION("Matching on storage kinds") {
    boolean b = sigma.boolean(true);
    unary n = negation(b);

    std::string s = n.match(
      [](negation) { return "negation"; },
      [](tomorrow) { return "tomorrow"; },
      [](w_tomorrow) { return "w_tomorrow"; },
      [](always) { return "always"; },
      [](eventually) { return "eventually"; }
    );

    REQUIRE(s == "negation");
  }

  SECTION("Matching with different syntaxes") {
    boolean b = sigma.boolean(true);
    formula n = negation(b);

    std::string s = n.match(
      [](boolean) { return "boolean"; },
      [](proposition) { return "proposition"; },
      [](unary) { return "unary"; },
      [](binary) { return "binary"; }
    );

    REQUIRE(s == "unary");
  }

  SECTION("Otherwise") { 
    formula f = sigma.boolean(true);

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

      conjunction c = (b && Y(p));

      auto [l, r] = c;

      REQUIRE(l == b);
      REQUIRE(r == Y(p));

      formula f = c;
      f.match(
        [&](conjunction conj, formula left, formula right) {
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
      std::vector<term> vars = {
        sigma.variable("x"), sigma.variable("y")
      };
      
      atom a = r(vars);

      formula f = a;
      f.match(
        [&](atom at, relation rel, auto const& terms) { 
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
      [](conjunction, auto ...args) {
        REQUIRE(sizeof...(args) == 2);
      },
      [](otherwise) {
        REQUIRE(false);
      }
    );
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

    boolean b = sigma.boolean("b");
    proposition p = sigma.proposition("p");
    unary u = !b;
    conjunction c = b && p;
    disjunction d = b || p;
    binary bin = c;
    formula f = u;

    REQUIRE_CT(b, p, formula);

    REQUIRE_CT(u, b, formula);
    REQUIRE_CT(bin, u, formula);
    REQUIRE_CT(f, u, formula);
    REQUIRE_CT(f, bin, formula);
    REQUIRE_CT(c, d, binary);
    REQUIRE_CT(c, bin, binary);
    REQUIRE_CT(b, c, formula);
    REQUIRE_CT(p, p, proposition);
    REQUIRE_CT(u, u, unary);
    REQUIRE_CT(f, f, formula);
  }
}
