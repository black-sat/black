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
using namespace black::internal::new_api;
using black::internal::identifier;

TEST_CASE("New API") {
  
  alphabet sigma;

  variable v = sigma.variable("x");
  function f{"f"};

  [[maybe_unused]]
  term t = f(v);

  boolean b = sigma.boolean(true);

  [[maybe_unused]]
  proposition p = sigma.proposition("hello");

  unary u = unary(unary::type::negation, b);

  REQUIRE(b.value());

  formula c = formula{conjunction(u, p)};

  std::optional<binary> c2 = c.to<binary>();
  REQUIRE(c2.has_value());
  REQUIRE(c.is<binary>());
  REQUIRE(!c.is<unary>());
  std::optional<unary> fail = c.to<unary>();
  REQUIRE(!fail.has_value());

  std::optional<conjunction> c3 = c.to<conjunction>();
  REQUIRE(c3.has_value());
  REQUIRE(c.is<conjunction>());

  static_assert(std::tuple_size_v<conjunction> == 2);
  static_assert(std::tuple_size_v<proposition> == 0);

  static_assert(std::is_same_v<std::tuple_element_t<0, conjunction>, formula>);

  auto [u2, p2] = conjunction(u, p);

  REQUIRE(u2 == u);
  REQUIRE(p2 == p);

  static_assert(std::is_same_v<std::common_type_t<binary, iff>, binary>);
  static_assert(std::is_same_v<std::common_type_t<binary, negation>, formula>);
  static_assert(std::is_same_v<std::common_type_t<tomorrow, negation>, unary>);
  static_assert(std::is_same_v<std::common_type_t<tomorrow, iff>, formula>);
  static_assert(std::is_same_v<std::common_type_t<tomorrow, binary>, formula>);
  static_assert(std::is_same_v<std::common_type_t<binary, tomorrow>, formula>);
  static_assert(std::is_same_v<std::common_type_t<tomorrow, formula>, formula>);
  static_assert(std::is_same_v<std::common_type_t<binary, formula>, formula>);
  static_assert(std::is_same_v<std::common_type_t<formula, binary>, formula>);
  static_assert(std::is_same_v<std::common_type_t<formula, tomorrow>, formula>);
}
