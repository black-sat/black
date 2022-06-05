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
#include <type_traits>

#define type_exists(Type, Syntax) \
  std::is_same_v< \
    Type, black::new_api::syntax::Type<black::new_api::syntax::Syntax> \
  >

TEST_CASE("Fragment namespaces") {

  black::new_api::alphabet sigma;

  SECTION("Top-level namespace") {
    using namespace black::new_api;
  
    static_assert(type_exists(formula, LTLPFO));
    static_assert(type_exists(term, LTLPFO));
    static_assert(type_exists(function, LTLPFO));
    static_assert(type_exists(relation, LTLPFO));
    static_assert(type_exists(unary, LTLPFO));
    static_assert(type_exists(binary, LTLPFO));

    formula f = sigma.proposition("p");

    REQUIRE(f.is<proposition>());
  }

  SECTION("Specific fragments") {
    using namespace black::new_api::FO;

    static_assert(type_exists(formula, FO));
    static_assert(type_exists(term, FO));
    static_assert(type_exists(function, FO));
    static_assert(type_exists(relation, FO));
    static_assert(type_exists(unary, FO));
    static_assert(type_exists(binary, FO));

    relation r = sigma.relation_symbol("r");
    variable x = sigma.variable("x");
    
    formula f = exists(x, r(x));

    REQUIRE(f.is<exists>());
  }
  
}