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

  unary<LTL> u = unary<LTL>(unary<LTL>::type::negation, b);

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
  REQUIRE(p2 == p);

  [[maybe_unused]] 
  known_func<LTLFO> func = sigma.negative();
  
  REQUIRE(func.is<negative>());

  auto i = sigma.function_symbol("ciao");

  REQUIRE(*i.label().get<std::string>() == "ciao");
  
  relation<FO> e = sigma.equal();

  REQUIRE(e.to<equal>().has_value());

  [[maybe_unused]]
  LTL::type<formula_accepts_type> t = 
    LTL::type<formula_accepts_type>::boolean;

  static_assert(type_list_contains<LTL::list, syntax_element::conjunction>);
  static_assert(!type_list_contains<LTL::list, syntax_element::historically>);
  static_assert(type_list_includes<LTL::list, Boolean::list>);
  static_assert(!type_list_includes<Boolean::list, LTL::list>);

  static_assert(is_syntax_allowed<Boolean, LTL>);
  static_assert(!is_syntax_allowed<LTL, Boolean>);

  [[maybe_unused]]
  formula<LTL> f20 = binary<LTL>(binary<LTL>::type::conjunction, u, p);

  [[maybe_unused]]
  formula<LTL> f10 = unary<LTL>(unary<LTL>::type::always, p);
  formula<LTL> f11 = always<LTL>(p);

  REQUIRE(f10 == f11);

  [[maybe_unused]]
  formula<Boolean> f12 = negation<Boolean>(p);

  [[maybe_unused]]
  unary<Boolean> f13 = negation<Boolean>(p);

  formula<LTL> f14 = negation<LTL>(formula<LTL>{p});

  REQUIRE(f14.is<negation<LTLP>>());
  REQUIRE(!f14.is<negation<Boolean>>());
  REQUIRE(f14.is<unary<LTLP>>());
  REQUIRE(!f14.is<unary<Boolean>>());
  REQUIRE(!f14.is<proposition>());

  struct props : make_fragment<syntax_element::proposition> { };
  struct untilprops : make_derived_fragment<props, syntax_element::until> { };

  [[maybe_unused]]
  formula<untilprops> f = until<untilprops>(p, p);


  [[maybe_unused]]
  tsl::hopscotch_map<variable, std::string> map;

  variable x = sigma.variable("x");
  [[maybe_unused]]
  formula<LTLFO> f30 = conjunction<LTLFO>(until<LTL>(p,p), forall<FO>(x, p));

  REQUIRE(x == f30.to<conjunction<LTLFO>>()->right().to<forall<LTLFO>>()->var());

  static_assert(!application_has_hierarchy_elements());
  static_assert(is_argument_allowed<function_symbol, FO>);
  
  function_symbol fs = sigma.function_symbol("f");
  relation_symbol rs = sigma.relation_symbol("r");

  [[maybe_unused]]
  term<FO> t40 = application<FO>(fs, std::vector{x});

  [[maybe_unused]]
  term<FO> t50 = fs(x);

  [[maybe_unused]]
  term<FO> t60 = fs(std::vector{x,x});
  
  [[maybe_unused]]
  formula<FO> a50 = rs(x);

  [[maybe_unused]]
  formula<FO> a60 = rs(std::vector{x,x});

  formula<LTL> f400 = sigma.boolean(true);
  formula<LTLFO> f500 = f400;

  f500 = rs(x);

  f500 = f400;

  unary<LTLFO> u100 = unary<LTLFO>(unary<LTLFO>::type::always, p);
  unary<LTL> u200 = unary<LTL>(unary<LTL>::type::negation, p);

  u100 = u200;
}
