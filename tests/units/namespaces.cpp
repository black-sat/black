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

#include <type_traits>

#define type_exists(Type, Syntax) \
  std::is_same_v< \
    Type, black::logic::Type<black::logic::Syntax> \
  >

TEST_CASE("Fragment namespaces") {

  black::logic::alphabet sigma;

  SECTION("Specific fragments") {
    using namespace black::logic::fragments::FO;

    static_assert(type_exists(formula, FO));
    static_assert(type_exists(term, FO));
    static_assert(type_exists(unary, FO));
    static_assert(type_exists(binary, FO));

    [[maybe_unused]]
    relation r = sigma.relation("r");
    variable x = sigma.variable("x");

    sort s = sigma.integer_sort();
    
    formula f = exists({x[s]}, r(x));

    REQUIRE(f.is<exists>());
  }

  SECTION("Only") {
    using namespace black::logic::fragments::LTL;

    proposition p = sigma.proposition("p");
    boolean b = sigma.boolean(true);

    formula f = always(p && b);

    STATIC_REQUIRE(black::logic::hierarchy<only<future>>);

    std::string s = f.match(
      [](boolean) { return "boolean"; },
      [](proposition) { return "proposition"; },
      [](only<future> o) { 
        return o.match(
          [](tomorrow) { return "tomorrow"; },
          [](w_tomorrow) { return "w_tomorrow"; },
          [](always) { return "always"; },
          [](eventually) { return "eventually"; },
          [](until) { return "until"; },
          [](release) { return "release"; },
          [](w_until) { return "w_until"; },
          [](s_release) { return "s_release"; }
        );
      },
      [](otherwise) { return "other"; }
    );

    REQUIRE(s == "always");
  }
  
}
