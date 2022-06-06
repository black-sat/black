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

using namespace std::literals;

using namespace black::new_api::syntax;

TEST_CASE("Pattern matching") {
  black::new_api::syntax::alphabet sigma;

  SECTION("Matching on formulas") {
    boolean b = sigma.boolean(true);
    formula<Boolean> n = negation<Boolean>(b);

    std::string s = n.match(
      [](boolean) { return "boolean"s; },
      [](proposition) { return "proposition"s; },
      [](unary<Boolean>, formula<Boolean> arg) { 
        return "unary<Boolean>("s + 
          arg.match(
            [](boolean) { return "boolean"; },
            [](proposition) { return "proposition"; },
            [](unary<Boolean>) { return "unary<Boolean>"; },
            [](binary<Boolean>) { return "binary<Boolean>"; } 
          ) + ")"; 
      },
      [](binary<Boolean>) { return "binary<Boolean>"s; }
    );

    REQUIRE(s == "unary<Boolean>(boolean)");
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
      [](only<Boolean, LTLP> o) {
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

  SECTION("Common type") {
    static_assert(
      std::is_same_v<
        std::common_type_t<formula<LTL>, proposition>, formula<LTL>
      >
    );
    static_assert(
      black::internal::new_api::is_syntax_allowed<
        std::common_type_t<formula<LTL>, unary<FO>>::syntax, LTLFO
      >
    );
    static_assert(
      black::internal::new_api::is_syntax_allowed<
        std::common_type_t<unary<LTL>, formula<FO>>::syntax, LTLFO
      >
    );
    static_assert(
      black::internal::new_api::is_syntax_allowed<
        std::common_type_t<unary<LTL>, negation<FO>>::syntax, LTLFO
      >
    );
    static_assert(
      black::internal::new_api::is_syntax_allowed<
        std::common_type_t<negation<LTL>, unary<FO>>::syntax, LTLFO
      >
    );
    static_assert(
      black::internal::new_api::is_syntax_allowed<
        std::common_type_t<negation<FO>, proposition>::syntax, FO
      >
    );
    static_assert(
      black::internal::new_api::is_syntax_allowed<
        std::common_type_t<tomorrow<LTL>, negation<FO>, proposition>::syntax, LTLFO
      >
    );
    static_assert(
      black::internal::new_api::is_syntax_allowed<
        std::common_type_t<proposition, unary<LTL>>::syntax, LTL
      >
    );
    static_assert(
      black::internal::new_api::is_syntax_allowed<
        std::common_type_t<negation<FO>, formula<LTL>>::syntax, LTLFO
      >
    );

    formula<LTLFO> f = sigma.proposition("p");
    auto f2 = f.match(
      [](boolean b) {
        return negation<FO>(b);
      },
      [](proposition p) {
        return p;
      },
      [&](otherwise) {
        return tomorrow<LTL>(sigma.boolean(false));
      }
    );

    REQUIRE(f2 == sigma.proposition("p"));
  }
  
}
