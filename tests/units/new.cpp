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

TEST_CASE("New API") {
  
  alphabet sigma;

  boolean b = sigma.boolean(true);

  proposition p = sigma.proposition("hello");

  unary u = unary<LTL>(unary<LTL>::type::negation, b);

  REQUIRE(b.value());

  formula<LTL> c = conjunction<LTL>(u, p);

  std::optional<binary<LTL>> c2 = c.to<binary<LTL>>();
  REQUIRE(c2.has_value());
  REQUIRE(c.is<binary<LTL>>());
  REQUIRE(!c.is<unary<LTL>>());
  std::optional<unary<LTL>> fail = c.to<unary<LTL>>();
  REQUIRE(!fail.has_value());

  std::optional<conjunction<LTL>> c3 = c.to<conjunction<LTL>>();
  REQUIRE(c3.has_value());
  REQUIRE(c.is<conjunction<LTL>>());

  static_assert(std::tuple_size_v<conjunction<LTL>> == 2);

  static_assert(std::is_same_v<std::tuple_element_t<0, conjunction<LTL>>, formula<LTL>>);

  auto [u2, p2] = conjunction<LTL>(u, p);

  REQUIRE(u2 == u);
  // REQUIRE(p2 == p);

  [[maybe_unused]] 
  function<LTL> func = sigma.negative();
  
  REQUIRE(func.is<negative>());

  auto i = sigma.uninterpreted("ciao");

  REQUIRE(*i.label().get<std::string>() == "ciao");
  
  relation<LTL> e = sigma.equal();

  REQUIRE(e.to<equal>().has_value());

  static_assert(is_type_allowed<equal,LTL>);

  using S = syntax<proposition, boolean, conjunction<void>>;

  proposition p7 = sigma.proposition("p");

  [[maybe_unused]]
  formula<S> f8 = p7;

  static_assert(are_types_allowed<S,S>);
  static_assert(are_types_allowed<S, LTL>);

  formula<S> f10 = disjunction<S>(p7, p7);
}